// Copyright (c) 2013 Technical University Braunschweig (Germany).
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
// Author(s):  Francisc Bungiu <fbungiu@gmail.com>
//             Michael Hemmer <michael.hemmer@cgal.org>

#include <CGAL/basic.h>
#include <CGAL/Cartesian.h>
#include <CGAL/Gmpq.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Simple_polygon_visibility_2.h>
#include <CGAL/Naive_visibility_2.h>
#include <CGAL/Triangular_expansion_visibility_2_.h>
#include <CGAL/test_model_methods.h>
#include <CGAL/test_utils.h>

#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {
{
	typedef CGAL::Gmpq                                Number_type;
  typedef CGAL::Cartesian<Number_type> 							Kernel;
  typedef CGAL::Arr_segment_traits_2<Kernel> 				Traits_2;
  typedef Traits_2::Point_2										      Point_2;
  typedef Traits_2::X_monotone_curve_2							Segment_2;
  typedef CGAL::Arrangement_2<Traits_2>							Arrangement_2;
  typedef CGAL::Simple_polygon_visibility_2<Arrangement_2, CGAL::Tag_false>
                                                    Simple_polygon_visibility_2;
  typedef CGAL::Naive_visibility_2<Arrangement_2, CGAL::Tag_false>
                                                    Naive_visibility_2;
  typedef CGAL::Triangular_expansion_visibility_2<Arrangement_2>
                                                    Triangular_expansion_visibility_2;

  if (argc == 2) {
    Simple_polygon_visibility_2 simple_visibility;
    Naive_visibility_2 naive_visibility;
    Triangular_expansion_visibility_2 triangular_visibility;
    const CGAL::Query_choice qchoice = CGAL::FACE;
    std::string input_arr_file(argv[1]);
    std::ifstream input(input_arr_file.c_str());
  //  CGAL::benchmark<Naive_visibility_2, Triangular_expansion_visibility_2>
    //              (naive_visibility, triangular_visibility, qchoice, input);
    CGAL::benchmark<Simple_polygon_visibility_2, Triangular_expansion_visibility_2>
                  (simple_visibility, triangular_visibility, qchoice, input);
  }
  else {
    std::cout << "Usage: ./benchmark [filename]\n";
    exit(0);
  }
}
	return 0;
}
