#include <vector>

#include "Scene_polygon_soup_item.h"
#include "Scene_polyhedron_item.h"
#include <CGAL/IO/Polyhedron_iostream.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>

#include <QObject>
#include <QtDebug>

#include <set>
#include <stack>
#include <algorithm>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

#include <CGAL/IO/OFF_reader.h>
#include <CGAL/IO/File_writer_OFF.h>
#include <CGAL/version.h>

#include <CGAL/orient_polygon_soup.h>
#include <CGAL/polygon_soup_to_polyhedron_3.h>
#include <CGAL/orient_polyhedron_3.h>



std::vector<float> positions_poly(0);
std::vector<float> positions_lines(0);
std::vector<float> positions_points(0);
std::vector<float> colors(0);
std::vector<float> normals(0);

GLuint rendering_program;
GLuint vertex_array_object;
int isInit = 0;
GLint location[2];
GLfloat *mvp_mat;
GLfloat *mv_mat;


GLuint vertex_shader;
GLuint fragment_shader;
GLuint program;
GLuint vao;
GLuint buffer[3];

struct light_info
{
    //position
    GLfloat pos_x;
    GLfloat pos_y;
    GLfloat pos_z;
    GLfloat alpha;

    //ambient
    GLfloat amb_x;
    GLfloat amb_y;
    GLfloat amb_z;
    GLfloat amb_alpha;
    //diffuse
    GLfloat diff_x;
    GLfloat diff_y;
    GLfloat diff_z;
    GLfloat diff_alpha;
    //specular
    GLfloat spec_x;
    GLfloat spec_y;
    GLfloat spec_z;
    GLfloat spec_alpha;

};
light_info light;
GLuint compile_shaders(void)
{



    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    //Generates an integer which will be used as ID for each buffer
    glGenBuffers(3, buffer);

    glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);

    glBufferData(GL_ARRAY_BUFFER, (positions_poly.size())*sizeof(positions_poly.data()), positions_poly.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, //number of the buffer
                          4, //number of floats to be taken
                          GL_FLOAT, // type of data
                          GL_FALSE, //not normalized
                          0, //compact data (not in a struct)
                          NULL //no offset (seperated in several buffers)
                          );
    glEnableVertexAttribArray(0);



    //Bind the second and initialize it
    glBindBuffer(GL_ARRAY_BUFFER, buffer[1]);
    glBufferData(GL_ARRAY_BUFFER, (colors.size())*sizeof(colors.data()), colors.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);

    //Bind the second and initialize it
    glBindBuffer(GL_ARRAY_BUFFER, buffer[2]);
    glBufferData(GL_ARRAY_BUFFER, (normals.size())*sizeof(normals.data()), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(2);



    //fill the vertex shader
    static const GLchar* vertex_shader_source[] =
    {
        "#version 300 es \n"
        " \n"
        "layout (location = 0) in vec4 positions_poly; \n"
        "layout (location = 1) in vec3 vColors; \n"
        "layout (location = 2) in vec3 vNormals; \n"
        "uniform mat4 mvp_matrix; \n"

        "uniform mat4 mv_matrix; \n"

        " vec3 light_pos =  vec3(0.0,0.0,1.0); \n"
        " vec3 light_diff = vColors; \n"
        " vec3 light_spec =  vec3(0.2,0.2,0.2); \n"
        " vec3 light_amb =  vec3(0.1,0.1,0.1); \n"
        " float spec_power = 0.128; \n"

        "out highp vec3 fColors; \n"
        "out highp vec3 fNormals; \n"
        " \n"

        "void main(void) \n"
        "{ \n"
        "vec4 P = mv_matrix * positions_poly; \n"
        "vec3 N = mat3(mv_matrix)* vNormals; \n"
        "vec3 L = light_pos - P.xyz; \n"
        "vec3 V = -P.xyz; \n"

        "N=normalize(N); \n"
        "L = normalize(L); \n"
        "V = normalize(V); \n"

        "vec3 R = reflect(-L, N); \n"

        "vec3 diffuse = max(dot(N,L), 0.0) * light_diff; \n"
        "vec3 specular = pow(max(dot(R,V), 0.0), spec_power) * light_spec; \n"

        "fColors = light_amb + diffuse + specular ; \n"

        "gl_Position = mvp_matrix * positions_poly; \n"
        "fNormals = vNormals; \n"
        "} \n"
    };
    //fill the fragment shader
    static const GLchar* fragment_shader_source[]=
    {
        "#version 300 es \n"
        " \n"
        "in highp vec3 fColors; \n"
        "in highp vec3 fNormals; \n"

        "out highp vec3 color; \n"
        " \n"
        "void main(void) \n"
        "{ \n"
        "/* Lighting gestion */ \n"
        " color = fColors; \n" //polygons de couleur rouge
        "} \n"
    };

    //creates and compiles the vertex shader
    vertex_shader =	glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, vertex_shader_source, NULL);
    glCompileShader(vertex_shader);

    GLint result;
    glGetShaderiv(vertex_shader,GL_COMPILE_STATUS,&result);
    if(result == GL_TRUE){
        std::cout<<"Vertex compilation OK"<<std::endl;
    } else {
        int maxLength;
        int length;
        glGetShaderiv(vertex_shader,GL_INFO_LOG_LENGTH,&maxLength);
        char* log = new char[maxLength];
        glGetShaderInfoLog(vertex_shader,maxLength,&length,log);
        std::cout<<"link error : Length = "<<length<<", log ="<<log<<std::endl;
    }
    fragment_shader =	glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, fragment_shader_source, NULL);
    glCompileShader(fragment_shader);

    glGetShaderiv(fragment_shader,GL_COMPILE_STATUS,&result);
    if(result == GL_TRUE){
        std::cout<<"Fragment compilation OK"<<std::endl;
    } else {
        int maxLength;
        int length;
        glGetShaderiv(fragment_shader,GL_INFO_LOG_LENGTH,&maxLength);
        char* log = new char[maxLength];
        glGetShaderInfoLog(fragment_shader,maxLength,&length,log);
        std::cout<<"link error : Length = "<<length<<", log ="<<log<<std::endl;
    }


    //creates the program, attaches and links the shaders
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glGetProgramiv(program,GL_LINK_STATUS,&result);
    if(result == GL_TRUE){
        std::cout<<"Link OK"<<std::endl;
    } else {
        int maxLength;
        int length;
        glGetProgramiv(program,GL_INFO_LOG_LENGTH,&maxLength);
        char* log = new char[maxLength];
        glGetProgramInfoLog(program,maxLength,&length,log);
        std::cout<<"link error : Length = "<<length<<", log ="<<log<<std::endl;
    }
    //Delete the shaders which are now in the memory
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    return program;
}

//The rendering function
void render()
{    
    if(isInit !=1)
        rendering_program = compile_shaders();

    const GLfloat color[] = { 0.9f, 0.9f, 0.9f, 1.0f };
    if(isInit !=1)
        glClearBufferfv(GL_COLOR, 0, color);

    // tells the GPU to use the program just created
    glUseProgram(rendering_program);

    //Allocates a uniform location for the MVP matrix
    location[0] = glGetUniformLocation(rendering_program, "mvp_matrix");
    location[1] = glGetUniformLocation(rendering_program, "mv_matrix");

    //Set the ModelViewProjection matrix
    glUniformMatrix4fv(location[0], 1, GL_FALSE, mvp_mat);
    glUniformMatrix4fv(location[1], 1, GL_FALSE, mv_mat);

    isInit = 1;
    //draw the polygons
    // the third argument is the number of vec4 that will be entered
    glDrawArrays(GL_TRIANGLES, 0, positions_poly.size()/4);
//glUseProgram(0);
}

struct Polyhedron_to_polygon_soup_writer {
    typedef Kernel::Point_3 Point_3;

    Polygon_soup* soup;
    Polygon_soup::Polygon_3 polygon;

    Polyhedron_to_polygon_soup_writer(Polygon_soup* soup) : soup(soup), polygon() {
    }

    void write_header( std::ostream&,
                       std::size_t /* vertices */,
                       std::size_t /* halfedges */,
                       std::size_t /* facets */,
                       bool /* normals */ = false ) {
        soup->clear();
    }

    void write_footer() {
    }

    void write_vertex( const double& x, const double& y, const double& z) {
        soup->points.push_back(Point_3(x, y, z));
    }

    void write_normal( const double& /* x */, const double& /* y */, const double& /* z */) {
    }

    void write_facet_header() {
    }

    void write_facet_begin( std::size_t no) {
        polygon.clear();
        polygon.reserve(no);
    }
    void write_facet_vertex_index( std::size_t index) {
        polygon.push_back(index);
    }
    void write_facet_end() {
        soup->polygons.push_back(polygon);
        polygon.clear();
    }
}; // end struct Polyhedron_to_soup_writer


Scene_polygon_soup_item::Scene_polygon_soup_item()
    : Scene_item(),
      soup(0),
      oriented(false)
{
    glewInit();
    mvp_mat = new GLfloat[16];
    mv_mat = new GLfloat[16];
    //gets the lighting info
    GLfloat result[4];
    glGetLightfv(GL_LIGHT0, GL_POSITION, result);
    light.pos_x=result[0];light.pos_y=result[1];light.pos_z=result[2];light.alpha=result[3];
    glGetLightfv(GL_LIGHT0, GL_AMBIENT, result);
    light.amb_x=result[0];light.amb_y=result[1];light.amb_z=result[2];light.amb_alpha=result[3];
}

Scene_polygon_soup_item::~Scene_polygon_soup_item()
{
    glDeleteVertexArrays(1, &vertex_array_object);
    glDeleteProgram(rendering_program);
    glDeleteVertexArrays(1, &vertex_array_object);

    delete soup;
}

Scene_polygon_soup_item*
Scene_polygon_soup_item::clone() const {
    Scene_polygon_soup_item* new_soup = new Scene_polygon_soup_item();
    new_soup->soup = soup->clone();
    new_soup->oriented = oriented;
    return new_soup;
}


bool
Scene_polygon_soup_item::load(std::istream& in)
{
    if (!soup) soup=new Polygon_soup();
    else soup->clear();
    return CGAL::read_OFF(in, soup->points, soup->polygons);
}

void Scene_polygon_soup_item::init_polygon_soup(std::size_t nb_pts, std::size_t nb_polygons){
    if(!soup)
        soup = new Polygon_soup;
    soup->clear();
    soup->points.reserve(nb_pts);
    soup->polygons.reserve(nb_polygons);
    oriented = false;
}

void Scene_polygon_soup_item::finalize_polygon_soup(){ soup->fill_edges(); }

#include <CGAL/IO/generic_print_polyhedron.h>
#include <iostream>

void Scene_polygon_soup_item::load(Scene_polyhedron_item* poly_item) {
    if(!poly_item) return;
    if(!poly_item->polyhedron()) return;

    if(!soup)
        soup = new Polygon_soup;

    Polyhedron_to_polygon_soup_writer writer(soup);
    CGAL::generic_print_polyhedron(std::cerr,
                                   *poly_item->polyhedron(),
                                   writer);
    emit changed();
}

void
Scene_polygon_soup_item::setDisplayNonManifoldEdges(const bool b)
{

    soup->display_non_manifold_edges = b;
    changed();
}

bool
Scene_polygon_soup_item::displayNonManifoldEdges() const {

    return soup->display_non_manifold_edges;
}

void Scene_polygon_soup_item::shuffle_orientations()
{
    for(Polygon_soup::size_type i = 0, end = soup->polygons.size();
        i < end; ++i)
    {
        if(std::rand() % 2 == 0) soup->inverse_orientation(i);
    }
    soup->fill_edges();
    changed();
}

void Scene_polygon_soup_item::inside_out()
{
    for(Polygon_soup::size_type i = 0, end = soup->polygons.size();
        i < end; ++i)
    {
        soup->inverse_orientation(i);
    }
    soup->fill_edges();
    changed();
}

bool
Scene_polygon_soup_item::orient()
{

    if(isEmpty() || this->oriented)
        return true; // nothing to do

    oriented = CGAL::orient_polygon_soup(soup->points, soup->polygons);
    return oriented;
}


bool
Scene_polygon_soup_item::save(std::ostream& out) const
{

    typedef Polygon_soup::size_type size_type;
    CGAL::File_writer_OFF writer;
    writer.write_header(out,
                        soup->points.size(),
                        0,
                        soup->polygons.size());
    for(size_type i = 0, end = soup->points.size();
        i < end; ++i)
    {
        const Point_3& p = soup->points[i];
        writer.write_vertex( p.x(), p.y(), p.z() );
    }
    writer.write_facet_header();
    for(size_type i = 0, end = soup->polygons.size();
        i < end; ++i)
    {
        const Polygon_soup::Polygon_3& polygon = soup->polygons[i];
        const size_type size = polygon.size();
        writer.write_facet_begin(size);
        for(size_type j = 0; j < size; ++j) {
            writer.write_facet_vertex_index(polygon[j]);
        }
        writer.write_facet_end();
    }
    writer.write_footer();

    return (bool) out;
}

bool
Scene_polygon_soup_item::exportAsPolyhedron(Polyhedron* out_polyhedron)
{
    orient();
    CGAL::polygon_soup_to_polyhedron_3(*out_polyhedron, soup->points, soup->polygons);

    if(out_polyhedron->size_of_vertices() > 0) {
        // Also check whether the consistent orientation is fine
        if(!CGAL::is_oriented(*out_polyhedron)) {
            out_polyhedron->inside_out();
        }
        return true;
    }
    return false;
}
QString
Scene_polygon_soup_item::toolTip() const
{

    if(!soup)
        return QString();

    return QObject::tr("<p><b>%1</b> (mode: %5, color: %6)<br />"
                       "<i>Polygons soup</i></p>"
                       "<p>Number of vertices: %2<br />"
                       "Number of polygons: %3</p>")
            .arg(this->name())
            .arg(soup->points.size())
            .arg(soup->polygons.size())
            .arg(this->renderingModeName())
            .arg(this->color().name());
}

void
Scene_polygon_soup_item::draw() const {
    std::cout<<"Erreur de Draw"<<std::endl;
    typedef Polygon_soup::Polygons::const_iterator Polygons_iterator;
    typedef Polygon_soup::Polygons::size_type size_type;
    if(isInit!=1)
    {
        for(Polygons_iterator it = soup->polygons.begin();
            it != soup->polygons.end(); ++it)
        {

            const Point_3& pa = soup->points[it->at(0)];
            const Point_3& pb = soup->points[it->at(1)];
            const Point_3& pc = soup->points[it->at(2)];

            Kernel::Vector_3 n = CGAL::cross_product(pb-pa, pc -pa);
            n = n / std::sqrt(n * n);
            //  ::glBegin(GL_POLYGON);
            //::glNormal3d(n.x(),n.y(),n.z());
            normals.push_back(n.x());
            normals.push_back(n.y());
            normals.push_back(n.z());

            colors.push_back(this->color().redF());
            colors.push_back(this->color().greenF());
            colors.push_back(this->color().blueF());

            normals.push_back(n.x());
            normals.push_back(n.y());
            normals.push_back(n.z());

            colors.push_back(this->color().redF());
            colors.push_back(this->color().greenF());
            colors.push_back(this->color().blueF());

            normals.push_back(n.x());
            normals.push_back(n.y());
            normals.push_back(n.z());

            colors.push_back(this->color().redF());
            colors.push_back(this->color().greenF());
            colors.push_back(this->color().blueF());

            for(size_type i = 0; i < it->size(); ++i)
            {
                const Point_3& p = soup->points[it->at(i)];

                positions_poly.push_back(p.x());
                positions_poly.push_back(p.y());
                positions_poly.push_back(p.z());
                positions_poly.push_back(1.0);


            }

        }

        if(soup->display_non_manifold_edges) {
            double current_color[4];
            GLboolean lightning;
            ::glGetDoublev(GL_CURRENT_COLOR, current_color);
            //::glColor3d(1., 0., 0.); // red
            ::glGetBooleanv(GL_LIGHTING, &lightning);
            ::glDisable(GL_LIGHTING);

            BOOST_FOREACH(const Polygon_soup::Edge& edge,
                          soup->non_manifold_edges)
            {
                const Point_3& a = soup->points[edge[0]];
                const Point_3& b = soup->points[edge[1]];

                //  ::glBegin(GL_LINES);
                // ::glVertex3d(a.x(), a.y(), a.z());
                // ::glVertex3d(b.x(), b.y(), b.z());
                // ::glEnd();
                positions_lines.push_back(a.x());
                positions_lines.push_back(a.y());
                positions_lines.push_back(a.y());

                positions_lines.push_back(b.x());
                positions_lines.push_back(b.y());
                positions_lines.push_back(b.y());

            }
            if(lightning) glEnable(GL_LIGHTING);
            //	::glColor4dv(current_color);
        }

    }
    render();
}

void
Scene_polygon_soup_item::draw(Viewer_interface* viewer) const {

    typedef Polygon_soup::Polygons::const_iterator Polygons_iterator;
    typedef Polygon_soup::Polygons::size_type size_type;

    //Remplit la matrice MVP
    GLdouble d_mvp_mat[16];
    viewer->camera()->getModelViewProjectionMatrix(d_mvp_mat);
    viewer->camera()->getModelViewMatrix(mv_mat);
    //Convert the GLdoubles matrix in GLfloats
    for (int i=0; i<16; ++i)
        mvp_mat[i] = GLfloat(d_mvp_mat[i]);



    if(isInit!=1)
    {
        for(Polygons_iterator it = soup->polygons.begin();
            it != soup->polygons.end(); ++it)
        {

            const Point_3& pa = soup->points[it->at(0)];
            const Point_3& pb = soup->points[it->at(1)];
            const Point_3& pc = soup->points[it->at(2)];

            Kernel::Vector_3 n = CGAL::cross_product(pb-pa, pc -pa);
            n = n / std::sqrt(n * n);
            //  ::glBegin(GL_POLYGON);
            //::glNormal3d(n.x(),n.y(),n.z());
            normals.push_back(n.x());
            normals.push_back(n.y());
            normals.push_back(n.z());

            colors.push_back(this->color().redF());
            colors.push_back(this->color().greenF());
            colors.push_back(this->color().blueF());

            normals.push_back(n.x());
            normals.push_back(n.y());
            normals.push_back(n.z());

            colors.push_back(this->color().redF());
            colors.push_back(this->color().greenF());
            colors.push_back(this->color().blueF());

            normals.push_back(n.x());
            normals.push_back(n.y());
            normals.push_back(n.z());

            colors.push_back(this->color().redF());
            colors.push_back(this->color().greenF());
            colors.push_back(this->color().blueF());

            for(size_type i = 0; i < it->size(); ++i)
            {
                const Point_3& p = soup->points[it->at(i)];

                positions_poly.push_back(p.x());
                positions_poly.push_back(p.y());
                positions_poly.push_back(p.z());
                positions_poly.push_back(1.0);


            }

        }

        if(soup->display_non_manifold_edges) {
            double current_color[4];
            GLboolean lightning;
            ::glGetDoublev(GL_CURRENT_COLOR, current_color);
            //::glColor3d(1., 0., 0.); // red
            ::glGetBooleanv(GL_LIGHTING, &lightning);
            ::glDisable(GL_LIGHTING);

            BOOST_FOREACH(const Polygon_soup::Edge& edge,
                          soup->non_manifold_edges)
            {
                const Point_3& a = soup->points[edge[0]];
                const Point_3& b = soup->points[edge[1]];

                //  ::glBegin(GL_LINES);
                // ::glVertex3d(a.x(), a.y(), a.z());
                // ::glVertex3d(b.x(), b.y(), b.z());
                // ::glEnd();
                positions_lines.push_back(a.x());
                positions_lines.push_back(a.y());
                positions_lines.push_back(a.y());

                positions_lines.push_back(b.x());
                positions_lines.push_back(b.y());
                positions_lines.push_back(b.y());

            }
            if(lightning) glEnable(GL_LIGHTING);
            //	::glColor4dv(current_color);
        }
    }

    render();
}

void
Scene_polygon_soup_item::draw_points() const {


    if(soup == 0) return;
    //::glBegin(GL_POINTS);
    for(Polygon_soup::Points::const_iterator pit = soup->points.begin(),
        end = soup->points.end();
        pit != end; ++pit)
    {
        // ::glVertex3d(pit->x(), pit->y(), pit->z());
        positions_points.push_back(pit->x());
        positions_points.push_back(pit->y());
        positions_points.push_back(pit->y());

    }
    //::glEnd();
}

bool
Scene_polygon_soup_item::isEmpty() const {

    return (soup == 0 || soup->points.empty());
}

Scene_polygon_soup_item::Bbox
Scene_polygon_soup_item::bbox() const {

    const Point_3& p = *(soup->points.begin());
    CGAL::Bbox_3 bbox(p.x(), p.y(), p.z(), p.x(), p.y(), p.z());
    for(Polygon_soup::Points::const_iterator it = soup->points.begin();
        it != soup->points.end();
        ++it) {
        bbox = bbox + it->bbox();
    }
    return Bbox(bbox.xmin(),bbox.ymin(),bbox.zmin(),
                bbox.xmax(),bbox.ymax(),bbox.zmax());
}

void
Scene_polygon_soup_item::new_vertex(const double& x,
                                    const double& y,
                                    const double& z)
{

    soup->points.push_back(Point_3(x, y, z));
}

void
Scene_polygon_soup_item::new_triangle(const std::size_t i,
                                      const std::size_t j,
                                      const std::size_t k)
{

    Polygon_soup::Polygon_3 new_polygon(3);
    new_polygon[0] = i;
    new_polygon[1] = j;
    new_polygon[2] = k;
    soup->polygons.push_back(new_polygon);
}

#include "Scene_polygon_soup_item.moc"
