// Copyright (c) 2002,2011 Utrecht University (The Netherlands).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
// You can redistribute it and/or modify it under the terms of the GNU
// General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
// 
//
// Author(s)     : Hans Tangelder (<hanst@cs.uu.nl>)


#ifndef CGAL_EUCLIDEAN_DISTANCE_H
#define CGAL_EUCLIDEAN_DISTANCE_H

#include <CGAL/Kd_tree_rectangle.h>
#include <CGAL/number_utils.h>
#include <CGAL/Dimension.h>
#include <boost/mpl/has_xxx.hpp>


namespace CGAL {

  template <class SearchTraits, class Query_type = typename SearchTraits::Point_d>
  class Euclidean_distance;

    
  namespace internal{
	    #ifndef HAS_DIMENSION_TAG
		#define HAS_DIMENSION_TAG
		BOOST_MPL_HAS_XXX_TRAIT_NAMED_DEF(has_dimension,Dimension,false);
		#endif

	  template <class SearchTraits, bool has_dim = has_dimension<SearchTraits>::value>
	  struct Euclidean_distance_base;

	  template <class SearchTraits>
	  struct Euclidean_distance_base<SearchTraits,true>{
		  typedef typename SearchTraits::Dimension Dimension;
	  };

	  template <class SearchTraits>
	  struct Euclidean_distance_base<SearchTraits,false>{
		  typedef Dynamic_dimension_tag Dimension;
	  };


    template <class SearchTraits>
    struct Spatial_searching_default_distance{
      typedef ::CGAL::Euclidean_distance<SearchTraits> type;
    };
  } //namespace internal

  template <class SearchTraits, class Query_type>
  class Euclidean_distance {
    
    SearchTraits traits;
    
    public:

    typedef typename SearchTraits::FT    FT;
    typedef typename SearchTraits::Point_d Point_d;
    typedef Point_d Query_item;

    typedef typename internal::Euclidean_distance_base<SearchTraits>::Dimension D;
	

    // default constructor
    Euclidean_distance(const SearchTraits& traits_=SearchTraits()):traits(traits_) {}

    
    inline FT transformed_distance(const Point_d& q, const Point_d& p) const {
        return transformed_distance(q,p, D());
    }

    //Dynamic version for runtime dimension
    inline FT transformed_distance(const Query_item& q, const Point_d& p, Dynamic_dimension_tag dt) const {
        FT distance = FT(0);
	typename SearchTraits::Construct_cartesian_const_iterator_d construct_it=traits.construct_cartesian_const_iterator_d_object();
        typename SearchTraits::Cartesian_const_iterator_d qit = construct_it(q),
	qe = construct_it(q,1), pit = construct_it(p);
	for(; qit != qe; qit++, pit++){
	    distance += ((*qit)-(*pit))*((*qit)-(*pit));
	}
        return distance;
    }

    //Generic version for DIM > 3
    template < int DIM >
    inline FT transformed_distance(const Query_item& q, const Point_d& p, Dimension_tag<DIM> dt) const {
        FT distance = FT(0);
        typename SearchTraits::Construct_cartesian_const_iterator_d construct_it=traits.construct_cartesian_const_iterator_d_object();
        typename SearchTraits::Cartesian_const_iterator_d qit = construct_it(q),
          qe = construct_it(q,1), pit = construct_it(p);
        for(; qit != qe; qit++, pit++){
	  distance += ((*qit)-(*pit))*((*qit)-(*pit));
        }
        return distance;
    }

    //DIM = 2 loop unrolled
    inline FT transformed_distance(const Query_item& q, const Point_d& p, Dimension_tag<2> dt) const {
        typename SearchTraits::Construct_cartesian_const_iterator_d construct_it=traits.construct_cartesian_const_iterator_d_object();
        typename SearchTraits::Cartesian_const_iterator_d qit = construct_it(q),pit = construct_it(p);
        FT distance = square(*qit - *pit);
        qit++;pit++;
        distance += square(*qit - *pit);
        return distance;
    }

    //DIM = 3 loop unrolled
    inline FT transformed_distance(const Query_item& q, const Point_d& p, Dimension_tag<3> dt) const {
        typename SearchTraits::Construct_cartesian_const_iterator_d construct_it=traits.construct_cartesian_const_iterator_d_object();
        typename SearchTraits::Cartesian_const_iterator_d qit = construct_it(q),pit = construct_it(p);
        FT distance = square(*qit - *pit);
        qit++;pit++;
        distance += square(*qit - *pit);
        qit++;pit++;
        distance += square(*qit - *pit);
        return distance;
    }

 


	inline FT min_distance_to_rectangle(const Query_item& q,
					    const Kd_tree_rectangle<FT,D>& r) const {
		FT distance = FT(0);
		typename SearchTraits::Construct_cartesian_const_iterator_d construct_it=traits.construct_cartesian_const_iterator_d_object();
                typename SearchTraits::Cartesian_const_iterator_d qit = construct_it(q),
		  qe = construct_it(q,1);
		for(unsigned int i = 0;qit != qe; i++, qit++){
		  if((*qit) < r.min_coord(i))
				distance += 
				(r.min_coord(i)-(*qit))*(r.min_coord(i)-(*qit));
		  else if ((*qit) > r.max_coord(i))
				distance +=  
				((*qit)-r.max_coord(i))*((*qit)-r.max_coord(i));
			
		}
		return distance;
	}

        inline FT min_distance_to_rectangle(const Query_item& q,
					    const Kd_tree_rectangle<FT,D>& r,std::vector<FT>& dists) {
		FT distance = FT(0);
		typename SearchTraits::Construct_cartesian_const_iterator_d construct_it=traits.construct_cartesian_const_iterator_d_object();
                typename SearchTraits::Cartesian_const_iterator_d qit = construct_it(q),
		  qe = construct_it(q,1);
		for(unsigned int i = 0;qit != qe; i++, qit++){
		  if((*qit) < r.min_coord(i)){
                                dists[i] = (r.min_coord(i)-(*qit));
				distance += dists[i] * dists[i];
                  }
		  else if ((*qit) > r.max_coord(i)){
                                dists[i] = ((*qit)-r.max_coord(i));
				distance +=  dists[i] * dists[i];
                  }
			
		}
		return distance;
	}

	inline FT max_distance_to_rectangle(const Query_item& q,
					     const Kd_tree_rectangle<FT,D>& r) const {
		FT distance=FT(0);
		typename SearchTraits::Construct_cartesian_const_iterator_d construct_it=traits.construct_cartesian_const_iterator_d_object();
                typename SearchTraits::Cartesian_const_iterator_d qit = construct_it(q),
		  qe = construct_it(q,1);
		for(unsigned int i = 0;qit != qe; i++, qit++){
				if ((*qit) <= (r.min_coord(i)+r.max_coord(i))/FT(2.0))
					distance += (r.max_coord(i)-(*qit))*(r.max_coord(i)-(*qit));
				else
					distance += ((*qit)-r.min_coord(i))*((*qit)-r.min_coord(i));
		};
		return distance;
	}

        inline FT max_distance_to_rectangle(const Query_item& q,
					     const Kd_tree_rectangle<FT,D>& r,std::vector<FT>& dists ) {
		FT distance=FT(0);
		typename SearchTraits::Construct_cartesian_const_iterator_d construct_it=traits.construct_cartesian_const_iterator_d_object();
                typename SearchTraits::Cartesian_const_iterator_d qit = construct_it(q),
		  qe = construct_it(q,1);
		for(unsigned int i = 0;qit != qe; i++, qit++){
				if ((*qit) <= (r.min_coord(i)+r.max_coord(i))/FT(2.0)){
                                        dists[i] = (r.max_coord(i)-(*qit));
					distance += dists[i] * dists[i];
                                }
				else{
                                        dists[i] = ((*qit)-r.min_coord(i));
					distance += dists[i] * dists[i];
                                }
		};
		return distance;
	}

	inline FT new_distance(FT dist, FT old_off, FT new_off,
			       int /* cutting_dimension */)  const {
		
		FT new_dist = dist + (new_off*new_off - old_off*old_off);
                return new_dist;
	}

  inline FT transformed_distance(FT d) const {
		return d*d;
	}

  inline FT inverse_of_transformed_distance(FT d) const {
		return CGAL::sqrt(d);
	}
  };

  //Functions for Rectangles
  template <class SearchTraits>
  class Euclidean_distance<SearchTraits, typename SearchTraits::Iso_box_d> {

    SearchTraits traits;
    
    public:

    typedef typename SearchTraits::FT    FT;
    typedef typename SearchTraits::Point_d Point_d;
    typedef typename SearchTraits::Iso_box_d Query_item;

    typedef typename internal::Euclidean_distance_base<SearchTraits>::Dimension D;

  inline FT transformed_distance(const typename SearchTraits::Iso_box_d& q, const Point_d& p) const {
		FT distance = FT(0);
		typename SearchTraits::Construct_cartesian_const_iterator_d construct_it=
                  traits.construct_cartesian_const_iterator_d_object();
		typename SearchTraits::Construct_min_vertex_d construct_min_vertex;
		typename SearchTraits::Construct_max_vertex_d construct_max_vertex;
                typename SearchTraits::Cartesian_const_iterator_d qmaxit = construct_it(construct_max_vertex(q)),
		  qe = construct_it(construct_max_vertex(q),1), qminit = construct_it(construct_min_vertex(q)),
		  pit = construct_it(p);
		for (; qmaxit != qe; ++pit,++qmaxit,++qminit) {
			if ((*pit)>(*qmaxit)) distance += 
			((*pit)-(*qmaxit)) * ((*pit)-(*qmaxit)); 
			else if ((*pit)<(*qminit)) distance += 
			((*qminit)-(*pit)) * ((*qminit)-(*pit));	
		}
        	return distance;
    }

  inline FT min_distance_to_rectangle(const typename SearchTraits::Iso_box_d& q,
					      const Kd_tree_rectangle<FT,D>& r) const {
		FT distance = FT(0);
		typename SearchTraits::Construct_cartesian_const_iterator_d construct_it=
                  traits.construct_cartesian_const_iterator_d_object();
		typename SearchTraits::Construct_min_vertex_d construct_min_vertex;
		typename SearchTraits::Construct_max_vertex_d construct_max_vertex;
                typename SearchTraits::Cartesian_const_iterator_d qmaxit = construct_it(construct_max_vertex(q)),
		  qe = construct_it(construct_max_vertex(q),1), qminit = construct_it(construct_min_vertex(q));
		for (unsigned int i = 0; qmaxit != qe; ++ qmaxit, ++qminit, ++i)  {
			if (r.min_coord(i)>(*qmaxit)) 
			  distance +=(r.min_coord(i)-(*qmaxit)) * (r.min_coord(i)-(*qmaxit)); 
			if (r.max_coord(i)<(*qminit)) 
			  distance += ((*qminit)-r.max_coord(i)) * ((*qminit)-r.max_coord(i));
	        }
		return distance;
	}

     inline FT min_distance_to_rectangle(const typename SearchTraits::Iso_box_d& q,
					      const Kd_tree_rectangle<FT,D>& r,std::vector<FT>& dists) {
		FT distance = FT(0);
		typename SearchTraits::Construct_cartesian_const_iterator_d construct_it=
                  traits.construct_cartesian_const_iterator_d_object();
		typename SearchTraits::Construct_min_vertex_d construct_min_vertex;
		typename SearchTraits::Construct_max_vertex_d construct_max_vertex;
                typename SearchTraits::Cartesian_const_iterator_d qmaxit = construct_it(construct_max_vertex(q)),
		  qe = construct_it(construct_max_vertex(q),1), qminit = construct_it(construct_min_vertex(q));
		for (unsigned int i = 0; qmaxit != qe; ++ qmaxit, ++qminit, ++i)  {
			if (r.min_coord(i)>(*qmaxit)) {
                          dists[i]=(r.min_coord(i)-(*qmaxit)); 
			  distance +=dists[i] * dists[i];
                        }
			if (r.max_coord(i)<(*qminit)) {
                          dists[i]=((*qminit)-r.max_coord(i));
			  distance += dists[i] * dists[i];
                        }
	        }
		return distance;
	}

    inline 
    FT 
    max_distance_to_rectangle(const typename SearchTraits::Iso_box_d& q,
			      const Kd_tree_rectangle<FT,D>& r) const {
      FT distance=FT(0);
      typename SearchTraits::Construct_cartesian_const_iterator_d construct_it=
        traits.construct_cartesian_const_iterator_d_object();
		typename SearchTraits::Construct_min_vertex_d construct_min_vertex;
		typename SearchTraits::Construct_max_vertex_d construct_max_vertex;
      typename SearchTraits::Cartesian_const_iterator_d qmaxit = construct_it(construct_max_vertex(q)),
	qe = construct_it(construct_max_vertex(q),1), qminit = construct_it(construct_min_vertex(q));
      for (unsigned int i = 0; qmaxit != qe; ++ qmaxit, ++qminit, ++i)  {
	if ( r.max_coord(i)-(*qminit) >(*qmaxit)-r.min_coord(i) )  
	  distance += (r.max_coord(i)-(*qminit)) * (r.max_coord(i)-(*qminit));
	else 
	  distance += ((*qmaxit)-r.min_coord(i)) * ((*qmaxit)-r.min_coord(i));
      }
      return distance;
    }

     inline 
    FT 
    max_distance_to_rectangle(const typename SearchTraits::Iso_box_d& q,
			      const Kd_tree_rectangle<FT,D>& r,std::vector<FT>& dists) {
      FT distance=FT(0);
      typename SearchTraits::Construct_cartesian_const_iterator_d construct_it=
        traits.construct_cartesian_const_iterator_d_object();
		typename SearchTraits::Construct_min_vertex_d construct_min_vertex;
		typename SearchTraits::Construct_max_vertex_d construct_max_vertex;
      typename SearchTraits::Cartesian_const_iterator_d qmaxit = construct_it(construct_max_vertex(q)),
	qe = construct_it(construct_max_vertex(q),1), qminit = construct_it(construct_min_vertex(q));
      for (unsigned int i = 0; qmaxit != qe; ++ qmaxit, ++qminit, ++i)  {
	if ( r.max_coord(i)-(*qminit) >(*qmaxit)-r.min_coord(i) )  {
          dists[i]=(r.max_coord(i)-(*qminit));
	  distance += dists[i] * dists[i];
        }
	else {
          dists[i]=((*qmaxit)-r.min_coord(i));
	  distance += dists[i] * dists[i];
        }
      }
      return distance;
    }

     inline FT new_distance(FT dist, FT old_off, FT new_off,
			       int /* cutting_dimension */)  const {
		
		FT new_dist = dist + (new_off*new_off - old_off*old_off);
                return new_dist;
	}

  inline FT transformed_distance(FT d) const {
		return d*d;
	}

  inline FT inverse_of_transformed_distance(FT d) const {
		return CGAL::sqrt(d);
	}

  }; // class Euclidean_distance

} // namespace CGAL
#endif // EUCLIDEAN_DISTANCE_H
