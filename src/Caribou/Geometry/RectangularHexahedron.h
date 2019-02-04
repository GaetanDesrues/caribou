#ifndef CARIBOU_GEOMETRY_RECTANGULARHEXAHEDRON_H
#define CARIBOU_GEOMETRY_RECTANGULARHEXAHEDRON_H

#include <Caribou/config.h>
#include <Caribou/Algebra/Vector.h>
#include <Caribou/Geometry/Node.h>
#include <Caribou/Geometry/Quad.h>
#include <Caribou/Geometry/Interpolation/Hexahedron.h>
#include <Caribou/Geometry/Internal/BaseHexahedron.h>

namespace caribou {
namespace geometry {

template <typename CanonicalElementType>
struct RectangularHexahedron : public internal::BaseHexahedron<CanonicalElementType, RectangularHexahedron<CanonicalElementType>>
{
    static constexpr INTEGER_TYPE NumberOfNodes = CanonicalElementType::NumberOfNodes;

    using Base = internal::BaseHexahedron<CanonicalElementType, RectangularHexahedron<CanonicalElementType>>;

    using NodeType = typename Base::NodeType;
    using QuadType = typename Base::QuadType;
    using Index = typename Base::Index;
    using Real = typename Base::Real;

    using LocalCoordinates = typename Base::LocalCoordinates;
    using WorldCoordinates = typename Base::WorldCoordinates;

    using Mat33 = algebra::Matrix<3, 3, Real>;
    using Size = algebra::Vector<3, Real>;

    constexpr
    RectangularHexahedron()
            : p_center {0, 0, 0}, p_H {2, 2, 2}, p_R (Mat33::Identity())
    {}

    constexpr
    RectangularHexahedron(const NodeType & center, const Size & dimensions, const Mat33 & rotation)
            : p_center (center), p_H (dimensions), p_R (rotation)
    {}

    constexpr
    RectangularHexahedron(const NodeType & center, const Size & dimensions)
            : p_center (center), p_H (dimensions), p_R (Mat33::Identity())
    {}

    /** Get the Node at given index */
    inline
    const NodeType
    node(Index index) const
    {
        const auto local_coordinates_of_node = CanonicalElementType::nodes[index];
        return T(local_coordinates_of_node);
    }

    /** Get the Node at given index */
    inline
    NodeType
    node(Index index)
    {
        const auto local_coordinates_of_node = CanonicalElementType::nodes[index];
        return T(local_coordinates_of_node);
    }

    /** Get a reference to the set of nodes */
    inline
    std::array<NodeType, NumberOfNodes>
    nodes() const
    {
        std::array<NodeType, NumberOfNodes> nodes;
        for (size_t i = 0; i < CanonicalElementType::nodes.size(); ++i)
            nodes[i] = node(i);
        return nodes;
    }

    /**
     * Get the local coordinates frame (a.k.a. the rotation matrix) positioned at the center of the hexahedron
     */
    inline
    Mat33
    frame() const
    {
        return p_R;
    }

    /**
     * Compute the jacobian matrix evaluated at local position {u,v,w}
     *
     * For a rectangular hexahedron, the jacobian matrix is constant and is defined as
     *
     *     1 | hx 0  0  |
     * J = - | 0  hy 0  |
     *     2 | 0  0  hz |
     *
     * where hx, hy, and hz are the dimension of the edges 0-1, 0-3 and 0-4 respectively.
     */
    inline
    algebra::Matrix<3, 3, Real>
    jacobian (const LocalCoordinates & /*coordinates*/) const
    {
        return jacobian();
    }

    /**
     * Compute the jacobian matrix.
     *
     * For a rectangular hexahedron, the jacobian matrix is constant and is defined as
     *
     *     1 | hx 0  0  |
     * J = - | 0  hy 0  |
     *     2 | 0  0  hz |
     *
     * where hx, hy, and hz are the dimension of the edges 0-1, 0-3 and 0-4 respectively.
     */
    inline
    algebra::Matrix<3, 3, Real>
    jacobian () const
    {
        return 1/2. * Mat33::Identity().direct_multiplication(p_H);
    }

    /**
     * Compute the transformation of a local position {u,v,w} to its world position {x,y,z}
     */
    inline
    WorldCoordinates
    T(const LocalCoordinates & coordinates) const
    {
        return p_R * coordinates.direct_multiplication(p_H/2.);
    }

    /**
     * Compute an integral approximation by gauss quadrature on the hexahedron of the given evaluation function.
     *
     * @example
     * \code{.cpp}
     * // Integrate the polynomial 1 + 2x + 2xy + 3*z on an hexahedron.
     * float result = RectangularHexahedron(x1, x2, x3, x4, x5, x6, x7, x8).gauss_integrate(
     *   [] (const RectangularHexahedron & hexa, const RectangularHexahedron::LocalCoordinates & coordinates) -> float {
     *     const auto & xi   = coordinates[0];
     *     const auto & eta  = coordinates[1];
     *     const auto & zeta = coordinates[2];
     *     return 1 + 2*xi + 2*xi*eta + 3*zeta;
     *   }
     * );
     * \endcode
     *
     * @tparam EvaluateFunctionType Callback function reference type. See f parameter.
     *
     * @param f
     * Callback function of the signature
     *
     *     ValueType f (const Hexahedron & hexa, const LocalCoordinates & coordinates);
     *
     * Where hexa is a reference to the current hexahadron on which we integrate, and the coordinates u, v and w
     * forms the local position of a sample point on which we want to get the evaluation value.
     *
     * @return The value of the integral computed on this hexahedron.
     *
     */
    template <typename EvaluateFunctor>
    inline
    auto
    gauss_quadrature(EvaluateFunctor f) const
    {
        static_assert(CanonicalElementType::gauss_nodes.size() == CanonicalElementType::gauss_weights.size(),
                      "Gauss nodes must have assigned weights.");

        // Constant for parallelepiped hexahedrons
        const auto detJ = jacobian().determinant();

        const auto p0 = CanonicalElementType::gauss_nodes[0];
        const auto w0 = CanonicalElementType::gauss_weights[0];
        const auto eval0 = f(*this, p0);
        auto result = eval0 * w0 * detJ;

        for (std::size_t i = 1; i < CanonicalElementType::gauss_nodes.size(); ++i) {
            const auto p = CanonicalElementType::gauss_nodes[i];
            const auto w = CanonicalElementType::gauss_weights[i];
            const auto eval = f(*this, p);
            result += eval * w * detJ;
        }

        return result;
    }

private:
    NodeType p_center; ///< Position of the center point of the hexahedron
    Size p_H; ///< Size of the hexahedron {hx, hy, hz}
    Mat33 p_R; ///< Rotation matrix (a.k.a. the local coordinates frame) at the center of the hexahedron
};

RectangularHexahedron() -> RectangularHexahedron<interpolation::Hexahedron8>;

} // namespace geometry
} // namespace caribou
#endif //CARIBOU_GEOMETRY_RECTANGULARHEXAHEDRON_H
