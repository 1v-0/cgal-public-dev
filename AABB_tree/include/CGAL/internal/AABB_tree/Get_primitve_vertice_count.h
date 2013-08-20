/*
 * Get_primitve_vertice_size.h
 *
 *  Created on: 19 ao�t 2013
 *      Author: jayalatn
 */

#ifndef GET_PRIMITVE_VERTICE_SIZE_H_
#define GET_PRIMITVE_VERTICE_SIZE_H_

template<typename K,typename PrimitiveType>
class Get_primitive_vertice_count
{

};

template<typename K>
class Get_primitive_vertice_count<K, typename K::Segment_2>
{
public:
   Get_primitive_vertice_count():number_of_vertices(2){};
   inline unsigned int operator()() const
   {

	   return number_of_vertices;
   }
private:
   unsigned int number_of_vertices;

};

template<typename K>
class Get_primitive_vertice_count<K, typename K::Triangle_2>
{
public:
   Get_primitive_vertice_count():number_of_vertices(3){};
   inline unsigned int operator()() const
   {

	   return number_of_vertices;
   }
private:
   unsigned int number_of_vertices;
};

template<typename K>
class Get_primitive_vertice_count<K, typename K::Segment_3>
{
public:
   Get_primitive_vertice_count():number_of_vertices(2){};
   inline unsigned int operator()() const
   {

	   return number_of_vertices;
   }
private:
   unsigned int number_of_vertices;

};

template<typename K>
class Get_primitive_vertice_count<K, typename K::Triangle_3>
{
public:
   Get_primitive_vertice_count():number_of_vertices(3){};
   inline unsigned int operator()() const
   {

	   return number_of_vertices;
   }
private:
   unsigned int number_of_vertices;

};


#endif /* GET_PRIMITVE_VERTICE_SIZE_H_ */
