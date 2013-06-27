#ifndef CGAL_CONSTRAINED_TRIANGULATION_FACE_BASE_2_2_H
#define CGAL_CONSTRAINED_TRIANGULATION_FACE_BASE_2_2_H

#include <CGAL/triangulation_assertions.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>

namespace CGAL {

}
template <class kernel,
					class Fbb = CGAL::Delaunay_mesh_face_base_2<kernel> >
class My_face_base : public Fbb
{
public:
	typedef typename Fbb Base;
	typedef typename Base::Vertex_handle Vertex_handle;
	typedef typename Base::Face_handle Face_handle;
	typedef typename std::pair<Face_handle, int> Edge;

	typedef typename CGAL::Point_2<kernel> Point;
	typedef typename CGAL::Segment_2<kernel> Segment;

public:
	enum {INSIDE = -1,
				UNDETERMINED = 0,
				OUTSIDE = 1};

private:
	// additional member data
	int m_location; // inside / outside / undetermined
	bool m_blind;
	Edge m_blinding_constraint;

public:
	template < typename TDS2 >
	struct Rebind_TDS {
		typedef typename Base::template Rebind_TDS<TDS2>::Other Fb2;
		typedef My_face_base<kernel,Fb2>                   Other;
	};

public:
	My_face_base()
		: Base(),
		  m_location(UNDETERMINED),
		  m_blind(false)
	{
	}
	My_face_base(Vertex_handle v1,
		           Vertex_handle v2,
		           Vertex_handle v3)
		: Base(v1,v2,v3),
		  m_location(UNDETERMINED),
		  m_blind(false)
	{
	}
	My_face_base(Vertex_handle v1,
		           Vertex_handle v2,
		           Vertex_handle v3,
		           Face_handle f1,
		           Face_handle f2,
		           Face_handle f3)
		: Base(v1,v2,v3,f1,f2,f3),
		  m_location(UNDETERMINED),
		  m_blind(false)
	{
	}
	My_face_base(Face_handle f)
		: My_face_base(f),
		  m_location(f->location()),
		  m_blind(false)
	{
	}

	My_face_base(Face_handle f,
		           Edge c)
		: My_face_base(f),
		  m_blind(true)
	{
		m_location = (f->location());
		constraint = c;
	}

	// inside/outside/undetermined
	const int location() const { return m_location; }
	int& location() { return m_location; }

	// sees its circumcenter or not?
	const bool& blind() const { return m_blind; }
	bool& blind(){ return m_blind; }

	// if blind, the constrained edge that prevents the face
	// to see its circumcenter 
	const Edge& blinding_constraint() const { return m_blinding_constraint; }
	Edge& blinding_constraint() { return m_blinding_constraint; }

};

} //namespace CGAL 
  
#endif //CGAL_CONSTRAINED_TRIANGULATION_FACE_BASE_2_H