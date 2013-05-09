#ifndef CGAL_INTERNAL_SURFACE_MESH_SEGMENTATION_AABB_TRAITS_H
#define CGAL_INTERNAL_SURFACE_MESH_SEGMENTATION_AABB_TRAITS_H

#include <CGAL/AABB_traits.h>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>

namespace CGAL{

/// @cond CGAL_DOCUMENT_INTERNAL
template<typename GeomTraits, typename AABB_primitive, bool fast_bbox_intersection>
class AABB_traits_SDF :
  public AABB_traits<GeomTraits, AABB_primitive>
{
  typedef AABB_traits<GeomTraits, AABB_primitive> Base_traits;

  class Do_intersect 
    : public Base_traits::Do_intersect
  {
  public:
    // not sure is it safe on templated funcs ? may be do not inherit and repeat functions...
    using Base_traits::Do_intersect::operator ();

    // activate functions below if K::FT is floating point and fast_bbox_intersection = true
    template <class K>
    typename boost::enable_if<
      boost::type_traits::ice_and<
        boost::is_floating_point<typename K::FT>::value,
        fast_bbox_intersection>,
      bool >::type
    operator()(const CGAL::Segment_3<K>& segment, const Bounding_box& bbox) const
    {
      const Point_3& p = segment.source(); 
      const Point_3& q = segment.target();

      return internal::do_intersect_bbox_segment_aux
        <double,
        true, // bounded at t=0 
        true, // bounded at t=1 
        false> // do not use static filters
        (p.x(), p.y(), p.z(),
        q.x(), q.y(), q.z(),
        bbox);
    }

    template <class K>
    typename boost::enable_if<
      boost::type_traits::ice_and<
        boost::is_floating_point<typename K::FT>::value,
        fast_bbox_intersection>,
      bool >::type
    operator()(const CGAL::Ray_3<K>& ray, const Bounding_box& bbox) const
    {
      const Point_3& p = ray.source(); 
      const Point_3& q = ray.second_point();

      return internal::do_intersect_bbox_segment_aux
        <double,
        true, // bounded at t=0 
        false,// not bounded at t=1 
        false> // do not use static filters
        (p.x(), p.y(), p.z(),
        q.x(), q.y(), q.z(),
        bbox);
    }

  };

public:
  Do_intersect do_intersect_object() {
    return Do_intersect();
  }
};
/// @endcond

} //namespace CGAL
#endif //CGAL_INTERNAL_SURFACE_MESH_SEGMENTATION_AABB_TRAITS_H