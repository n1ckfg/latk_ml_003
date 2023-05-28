#ifndef CROSSSTRUCTURES_H
#define CROSSSTRUCTURES_H

#include <unordered_map>
#include <Eigen/Core>

/**\file CrossStructures.h
 * \authour Bastien Wailly <bastien.wailly@inria.fr>, Inria
 * \brief Structures used for cross vertices and cross edges, when computing face contours. */

/**\typedef CrossVertex
 * \brief A CrossVertex is a vertex lying on a mesh edge. It has interpolated gradient and normal information.*/
typedef struct _crossVertex {
    Eigen::RowVector3d position;
    double valueThreshold;
    
    Eigen::RowVector3d gradient;
    Eigen::RowVector3d normal;
    
    int edge_index;

    bool operator== (const struct _crossVertex &other) const {
        return position == other.position;
    }
} CrossVertex;

namespace std
{
    /**\struct hash<CrossVertex>
     * \brief Defining hash of a CrossVertex for use in an std::unordered_set */
    template<>
    struct hash<CrossVertex> {
        size_t operator() (const CrossVertex &crossVertex) const {
            const Eigen::RowVector3d &position = crossVertex.position;
            size_t hash_position_x = hash<double>{}(position(0));
            size_t hash_position_y = hash<double>{}(position(1));
            size_t hash_position_z = hash<double>{}(position(2));

            size_t final_hash = hash_position_x ^ (hash_position_y << 1);
            return final_hash ^ (hash_position_z << 1);
        }
    };
}

/**\enum CrossEdgeType
 * \brief A CrossEdge can be of two types, depending on the number of cross vertices of the face [Ohtake 2004] */
enum class CrossEdgeType { 
    REGULAR, /*!<@brief 2 cross vertices on the face = we create a contour joining them. */
    CENTROID /*!<@brief 3 cross vertices on the face = we create 3 contours joining each with the center of the face */
};
using CrossVertexPair = std::pair<CrossVertex, CrossVertex>;
    
/**\typedef CrossEdge
 * \brief A CrossEdge is a line between two CrossVertex, lying on a face */
typedef struct _crossEdge {
    CrossVertexPair vertices;
    int face_index;
    CrossEdgeType type;
    int edges[3];
} CrossEdge;

/**\brief map from mesh edge index to cross vertex */
using CrossVerticesMap = std::unordered_map<unsigned int, CrossVertex>;

#endif /* CROSSSTRUCTURES_H */

