#include "Polyhedron_demo_plugin_helper.h"
#include "Polyhedron_demo_plugin_interface.h"

#include "ui_Mesh_segmentation_widget.h"
#include "Scene_polyhedron_item.h"
#include "Polyhedron_type.h"
#include "Scene.h"
#include <CGAL/Surface_mesh_segmentation.h>
#include <QApplication>
#include <QMainWindow>
#include <QInputDialog>
#include <QTime>
#include <QAction>
#include <QDebug>
#include <QObject>
#include <QDockWidget>
//#include <QtConcurrentRun>
#include <map>
#include <algorithm>

#include <boost/property_map/property_map.hpp>

class Polyhedron_demo_mesh_segmentation_plugin : 
    public QObject,
    public Polyhedron_demo_plugin_helper
{
    Q_OBJECT
        Q_INTERFACES(Polyhedron_demo_plugin_interface)
private:
    typedef CGAL::Surface_mesh_segmentation<Polyhedron> Segmentation;
    typedef std::map<Scene_polyhedron_item*, Segmentation> Item_functor_map;
public:

    QList<QAction*> actions() const {
        return QList<QAction*>() << actionSegmentation;
    }

    bool applicable() const {
      return 
        qobject_cast<Scene_polyhedron_item*>(scene->item(scene->mainSelectionIndex()));
    }    
    
    void init(QMainWindow* mainWindow, Scene_interface* scene_interface) {
        this->scene = scene_interface;
        this->mw = mainWindow;
        actionSegmentation = new QAction("Mesh Segmentation", mw);
        if(actionSegmentation) {
            connect(actionSegmentation, SIGNAL(triggered()),this, SLOT(on_actionSegmentation_triggered()));
        }
        
        // adding slot for itemAboutToBeDestroyed signal, aim is removing item from item-functor map.
        Scene* scene = dynamic_cast<Scene*>(scene_interface);
        if(scene)
        {
            connect(scene, SIGNAL(itemAboutToBeDestroyed(Scene_item*)),this, SLOT(itemAboutToBeDestroyed(Scene_item*)));
        }
    }
    
    void colorize(Polyhedron* polyhedron, Segmentation& segmentation, bool sdf);
    public slots:
        void on_actionSegmentation_triggered();
        void on_Partition_button_clicked();
        void on_SDF_button_clicked();
        void itemAboutToBeDestroyed(Scene_item*);
private:
    QAction* actionSegmentation;

    QDockWidget* dock_widget;
    Ui::Mesh_segmentation_widget* ui_widget;
    
    Item_functor_map item_functor_map;
};

void Polyhedron_demo_mesh_segmentation_plugin::itemAboutToBeDestroyed(Scene_item* scene_item)
{
    Scene_polyhedron_item* item = qobject_cast<Scene_polyhedron_item*>(scene_item);
    if(!item) { return; }
    item_functor_map.erase(item);    
}
void Polyhedron_demo_mesh_segmentation_plugin::on_actionSegmentation_triggered()
{    
    dock_widget = new QDockWidget("Mesh segmentation parameters", mw);
    
    ui_widget = new Ui::Mesh_segmentation_widget();
    QWidget* qw =new QWidget();
    ui_widget->setupUi(qw); //calling this on dock_widget is not working, since dock_widget has already layout
    // deleting dock_widget layout is also not working. So for a work-around I created a intermadiate widget (qw).
    dock_widget->setWidget(qw); // transfer widgets in ui_widget by qw.
    mw->addDockWidget(Qt::LeftDockWidgetArea, dock_widget);
    
    connect(ui_widget->Partition_button,  SIGNAL(clicked()), this, SLOT(on_Partition_button_clicked()));   
    connect(ui_widget->SDF_button,  SIGNAL(clicked()), this, SLOT(on_SDF_button_clicked()));  
    dock_widget->show();   
}

void Polyhedron_demo_mesh_segmentation_plugin::on_SDF_button_clicked()
{   
    Scene_interface::Item_id index = scene->mainSelectionIndex();
    Scene_polyhedron_item* item = qobject_cast<Scene_polyhedron_item*>(scene->item(index));
    if(!item) { return; }
    QApplication::setOverrideCursor(Qt::WaitCursor);
    
    int number_of_rays = ui_widget->Number_of_rays_spin_box->value();
    double cone_angle = (ui_widget->Cone_angle_spin_box->value()  / 180.0) * CGAL_PI;  
    bool create_new_item = ui_widget->New_item_check_box->isChecked();
    
    Scene_polyhedron_item* new_item = NULL;
    Item_functor_map::iterator pair;
    if(create_new_item)
    {
        // create new item
        new_item = new Scene_polyhedron_item(*item->polyhedron()); 
        new_item->setGouraudMode();
        new_item->setName(tr("%1 (SDF-%2-%3)").arg(item->name()).arg(number_of_rays).arg(ui_widget->Cone_angle_spin_box->value()));
        item->setVisible(false);             
        index = scene->addItem(new_item); 
        
        // create new functor - and add it to map
        pair = item_functor_map.insert(
                std::pair<Scene_polyhedron_item*, Segmentation>(new_item, Segmentation(*new_item->polyhedron()))).first;   
    }
    else
    {
        new_item = item;
        Item_functor_map::iterator it = item_functor_map.find(new_item);
        if(it == item_functor_map.end())
        {
            // create new functor, because there are none.
            pair = item_functor_map.insert(
                std::pair<Scene_polyhedron_item*, Segmentation>(new_item, Segmentation(*new_item->polyhedron()))).first;              
        }
        else
        {
            pair = it;
        }
    }  
    pair->second.calculate_sdf_values(cone_angle, number_of_rays);
    colorize(pair->first->polyhedron(), pair->second, true);
    scene->setSelectedItem(index);
    scene->itemChanged(pair->first);
    
    QApplication::restoreOverrideCursor();   
}

void Polyhedron_demo_mesh_segmentation_plugin::on_Partition_button_clicked()
{    
    Scene_interface::Item_id index = scene->mainSelectionIndex();
    Scene_polyhedron_item* item = qobject_cast<Scene_polyhedron_item*>(scene->item(index));
    if(!item) { return; }
    
    QApplication::setOverrideCursor(Qt::WaitCursor);
    
    int number_of_clusters = ui_widget->Number_of_clusters_spin_box->value();   
    double smoothness = ui_widget->Smoothness_spin_box->value();
    int number_of_rays = ui_widget->Number_of_rays_spin_box->value();
    double cone_angle = (ui_widget->Cone_angle_spin_box->value()  / 180.0) * CGAL_PI;  
    bool create_new_item = ui_widget->New_item_check_box->isChecked();   
    
    Scene_polyhedron_item* new_item = NULL;
    Item_functor_map::iterator pair;
    if(create_new_item)
    {
        // create new item
        new_item = new Scene_polyhedron_item(*item->polyhedron()); 
        new_item->setGouraudMode();
        new_item->setName(tr("%1 (Segmentation-%2-%3)").arg(item->name()).arg(number_of_clusters).arg(smoothness));
        item->setVisible(false);             
        index = scene->addItem(new_item); 
        
        // create new functor
        Item_functor_map::iterator it = item_functor_map.find(item);
        
        if(it != item_functor_map.end())
        {         
            // copy state of the existent functor to new functor, which uses new polyhedron
            pair = item_functor_map.insert(
                std::pair<Scene_polyhedron_item*, Segmentation>(new_item, Segmentation(*new_item->polyhedron(), it->second))).first;  
        }
        else
        {
            pair = item_functor_map.insert(
                std::pair<Scene_polyhedron_item*, Segmentation>(new_item, Segmentation(*new_item->polyhedron()))).first; 
        }           
    }
    else
    {
        new_item = item;
        Item_functor_map::iterator it = item_functor_map.find(new_item);
        if(it == item_functor_map.end())
        {
            // create new functor, because there are none.
            pair = item_functor_map.insert(
                std::pair<Scene_polyhedron_item*, Segmentation>(new_item, Segmentation(*new_item->polyhedron()))).first;             
            pair->second.calculate_sdf_values(cone_angle, number_of_rays);            
        }
        else
        {
            pair = it;
        }
    }  
    pair->second.partition(number_of_clusters, smoothness);
    //colorize(pair->first->polyhedron(), pair->second, false);
    scene->setSelectedItem(index);
    scene->itemChanged(pair->first);  
    QApplication::restoreOverrideCursor();

}

template <typename PairType> 
struct compare_pairs_by_second
{
    bool operator()(const PairType& v1, const PairType& v2) const
    { return v1.second < v2.second; }
};

void Polyhedron_demo_mesh_segmentation_plugin::colorize(
     Polyhedron*   polyhedron,
     Segmentation& segmentation,     
     bool sdf)
{
    int patch_id = 0;
    Polyhedron::Facet_iterator facet_it = polyhedron->facets_begin();
    
    std::vector<std::pair<Polyhedron::Facet_iterator, double> > facet_sdf_values;
    for(Segmentation::Facet_const_iterator facet_const_it = segmentation.mesh.facets_begin(); 
        facet_const_it != segmentation.mesh.facets_end(); ++facet_const_it, ++facet_it)   
    {
        if(sdf)
        {
            double sdf_value = segmentation.get_sdf_value_of_facet(facet_const_it); 
            facet_sdf_values.push_back(std::pair<Polyhedron::Facet_iterator, double>(facet_it, sdf_value));                 
        }
        else
        {
            int segment_id = segmentation.get_segment_id_of_facet(facet_const_it);
            facet_it->set_patch_id(segment_id);                       
        }        
    }
    if(sdf)
    {   compare_pairs_by_second<std::pair<Polyhedron::Facet_iterator, double> > cmp;
        std::sort(facet_sdf_values.begin(), facet_sdf_values.end(), cmp);
        patch_id = 0;
        for(std::vector<std::pair<Polyhedron::Facet_iterator, double> >::iterator pair_it = facet_sdf_values.begin();
            pair_it != facet_sdf_values.end(); ++pair_it)
        {
            pair_it->first->set_patch_id(patch_id++);
        }
    }
}


Q_EXPORT_PLUGIN2(Polyhedron_demo_mesh_segmentation_plugin, Polyhedron_demo_mesh_segmentation_plugin)

#include "Polyhedron_demo_mesh_segmentation_plugin.moc"
