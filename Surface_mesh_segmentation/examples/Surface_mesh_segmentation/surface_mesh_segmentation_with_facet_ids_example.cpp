#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Polyhedron_items_with_id_3.h>
#include <CGAL/IO/Polyhedron_iostream.h>
#include <CGAL/mesh_segmentation.h>

#include <boost/property_map/property_map.hpp>

#include <iostream>
#include <fstream>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Polyhedron_3<Kernel, CGAL::Polyhedron_items_with_id_3>  Polyhedron;

// Custom implementation of LvaluePropertyMap.
// It uses std::vector as underlying data structure, and associates data with facets
// by using id field of facets as indices for data.
template<class PolyhedronWithId, class ValueType>
struct Polyhedron_with_id_to_vector_property_map
    : public boost::put_get_helper<ValueType&,
             Polyhedron_with_id_to_vector_property_map<PolyhedronWithId, ValueType> >
{
public:
    typedef typename PolyhedronWithId::Facet_const_handle key_type;
    typedef ValueType value_type;
    typedef value_type& reference;
    typedef boost::lvalue_property_map_tag category;

    Polyhedron_with_id_to_vector_property_map() : internal_vector(NULL) { }
    Polyhedron_with_id_to_vector_property_map(std::vector<ValueType>* internal_vector)
         : internal_vector(internal_vector) { }
        
    reference operator[](key_type key) const { return (*internal_vector)[key->id()]; }
private:
    std::vector<ValueType>* internal_vector;
};

int main(int argc, char **argv)
{
    if (argc !=2){
        std::cerr << "Usage: " << argv[0] << " input.off\n";
        return 1;
    }
  
    // create and read Polyhedron
    Polyhedron mesh;
    std::ifstream input(argv[1]);
    
    if ( !input || !(input >> mesh) || mesh.empty() ){
        std::cerr << argv[1] << " is not a valid off file.\n";
        return 1;
    }
	// assign id field for each facet	
	int facet_id = 0;
    for(Polyhedron::Facet_iterator facet_it = mesh.facets_begin(); facet_it != mesh.facets_end(); 
        ++facet_it, ++facet_id)
    {
        facet_it->id() = facet_id;
    }
    // create a property-map for sdf values
	std::vector<double> sdf_values(mesh.size_of_facets()); 
	Polyhedron_with_id_to_vector_property_map<Polyhedron, double> sdf_property_map(&sdf_values);

    CGAL::sdf_values_computation(mesh, sdf_property_map);
	// access sdf values (with constant-complexity) either via sdf_values or sdf_property_map
	for(Polyhedron::Facet_const_iterator facet_it = mesh.facets_begin(); 
		facet_it != mesh.facets_end(); ++facet_it)
    {
		std::cout << (sdf_property_map[facet_it] == sdf_values[facet_it->id()]) << std::endl;
	}
    // create a property-map for segment-ids 
	std::vector<double> segment_ids(mesh.size_of_facets()); 
	Polyhedron_with_id_to_vector_property_map<Polyhedron, double> segment_property_map(&segment_ids);

    CGAL::surface_mesh_segmentation_from_sdf_values(mesh, sdf_property_map, segment_property_map);

    // access segment-ids (with constant-complexity) either via segment_ids or segment_property_map
    for(Polyhedron::Facet_const_iterator facet_it = mesh.facets_begin(); 
		facet_it != mesh.facets_end(); ++facet_it)   
    {
        std::cout << (segment_property_map[facet_it] == segment_ids[facet_it->id()]) << std::endl;                                
    }
}