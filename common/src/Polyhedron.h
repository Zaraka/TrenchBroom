/*
 Copyright (C) 2010-2014 Kristian Duske
 
 This file is part of TrenchBroom.
 
 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_Polyhedron_h
#define TrenchBroom_Polyhedron_h

#include "Allocator.h"
#include "VecMath.h"
#include "DoublyLinkedList.h"

#include <cassert>
#include <queue>
#include <vector>

template <typename T, typename FP>
class Polyhedron {
    typedef Vec<T,3> V;
    typedef typename Vec<T,3>::List PosList;
public:
    class Vertex;
    class Edge;
    class HalfEdge;
    class Face;
private:
    class GetVertexLink {
    public:
        typename DoublyLinkedList<Vertex, GetVertexLink>::Link& operator()(Vertex* vertex) const;
        const typename DoublyLinkedList<Vertex, GetVertexLink>::Link& operator()(const Vertex* vertex) const;
    };
    
    class GetEdgeLink {
    public:
        typename DoublyLinkedList<Edge, GetEdgeLink>::Link& operator()(Edge* edge) const;
        const typename DoublyLinkedList<Edge, GetEdgeLink>::Link& operator()(const Edge* edge) const;
    };

    class GetHalfEdgeLink {
    public:
        typename DoublyLinkedList<HalfEdge, GetHalfEdgeLink>::Link& operator()(HalfEdge* halfEdge) const;
        const typename DoublyLinkedList<HalfEdge, GetHalfEdgeLink>::Link& operator()(const HalfEdge* halfEdge) const;
    };

    class GetFaceLink {
    public:
        typename DoublyLinkedList<Face, GetFaceLink>::Link& operator()(Face* face) const;
        const typename DoublyLinkedList<Face, GetFaceLink>::Link& operator()(const Face* face) const;
    };
    
    typedef typename DoublyLinkedList<Vertex, GetVertexLink>::Link VertexLink;
    typedef typename DoublyLinkedList<Edge, GetEdgeLink>::Link EdgeLink;
    typedef typename DoublyLinkedList<HalfEdge, GetHalfEdgeLink>::Link HalfEdgeLink;
    typedef typename DoublyLinkedList<Face, GetFaceLink>::Link FaceLink;
public:
    typedef DoublyLinkedList<Vertex, GetVertexLink> VertexList;
    typedef DoublyLinkedList<Edge, GetEdgeLink> EdgeList;
    typedef DoublyLinkedList<HalfEdge, GetHalfEdgeLink> HalfEdgeList;
    typedef DoublyLinkedList<Face, GetFaceLink> FaceList;
public:
    class Vertex : public Allocator<Vertex> {
    private:
        friend class Polyhedron<T,FP>;
        
        V m_position;
        VertexLink m_link;
        HalfEdge* m_leaving;
    private:
        Vertex(const V& position);
    public:
        const V& position() const;
        Vertex* next() const;
        Vertex* previous() const;
        HalfEdge* leaving() const;
    private:
        HalfEdge* findConnectingEdge(const Vertex* vertex) const;
        HalfEdge* findColinearEdge(const HalfEdge* arriving) const;
        void setPosition(const V& position);
        void setLeaving(HalfEdge* edge);
    };

    class Edge : public Allocator<Edge> {
    private:
        friend class Polyhedron<T,FP>;
        
        HalfEdge* m_first;
        HalfEdge* m_second;
        EdgeLink m_link;
    private:
        Edge(HalfEdge* first, HalfEdge* second = NULL);
    public:
        Vertex* firstVertex() const;
        Vertex* secondVertex() const;
        Vertex* otherVertex(Vertex* vertex) const;
        HalfEdge* firstEdge() const;
        HalfEdge* secondEdge() const;
        HalfEdge* twin(const HalfEdge* halfEdge) const;
        V vector() const;
        V center() const;
        Face* firstFace() const;
        Face* secondFace() const;
        Vertex* commonVertex(const Edge* other) const;
        bool hasVertex(const Vertex* vertex) const;
        bool hasPosition(const V& position, T epsilon = Math::Constants<T>::almostZero()) const;
        bool hasPositions(const V& position1, const V& position2, T epsilon = Math::Constants<T>::almostZero()) const;
        bool fullySpecified() const;
        bool contains(const V& point, T maxDistance = Math::Constants<T>::almostZero()) const;
        Edge* next() const;
        Edge* previous() const;
    private:
        Edge* split(const Plane<T,3>& plane);
        Edge* splitAtCenter();
        Edge* insertVertex(const V& position);
        void flip();
        void makeFirstEdge(HalfEdge* edge);
        void makeSecondEdge(HalfEdge* edge);
        void setFirstAsLeaving();
        void unsetSecondEdge();
        void setSecondEdge(HalfEdge* second);
    };
    
    class HalfEdge : public Allocator<HalfEdge> {
    private:
        friend class Polyhedron<T,FP>;
        
        Vertex* m_origin;
        Edge* m_edge;
        Face* m_face;
        HalfEdgeLink m_link;
    private:
        HalfEdge(Vertex* origin);
    public:
        ~HalfEdge();
    public:
        Vertex* origin() const;
        Vertex* destination() const;
        T length() const;
        T squaredLength() const;
        V vector() const;
        Edge* edge() const;
        Face* face() const;
        HalfEdge* next() const;
        HalfEdge* previous() const;
        HalfEdge* twin() const;
        HalfEdge* previousIncident() const;
        HalfEdge* nextIncident() const;
        bool hasOrigins(const typename V::List& positions, T epsilon = Math::Constants<T>::almostZero()) const;
        String asString() const;
    private:
        bool isLeavingEdge() const;
        bool colinear(const HalfEdge* other) const;
        void setOrigin(Vertex* origin);
        void setEdge(Edge* edge);
        void setFace(Face* face);
        void setAsLeaving();
    };

    class Face : public Allocator<Face> {
    private:
        friend class Polyhedron<T,FP>;
        
        // Boundary is counter clockwise.
        HalfEdgeList m_boundary;
        FP* m_payload;
        FaceLink m_link;
    private:
        Face(HalfEdgeList& boundary);
    public:
        FP* payload() const;
        void setPayload(FP* payload);
        Face* next() const;
        Face* previous() const;
        size_t vertexCount() const;
        const HalfEdgeList& boundary() const;
        V origin() const;
        bool hasPositions(const typename V::List& positions, T epsilon = Math::Constants<T>::almostZero()) const;
        V normal() const;
        V center() const;
        T intersectWithRay(const Ray<T,3>& ray, const Math::Side side) const;
    private:
        bool visibleFrom(const V& point) const;
        bool coplanar(const Face* other) const;
        Math::PointStatus::Type pointStatus(const V& point, T epsilon = Math::Constants<T>::pointStatusEpsilon()) const;
        void flip();
        void insertIntoBoundaryBefore(HalfEdge* before, HalfEdge* edge);
        void insertIntoBoundaryAfter(HalfEdge* after, HalfEdge* edge);
        size_t removeFromBoundary(HalfEdge* from, HalfEdge* to);
        size_t removeFromBoundary(HalfEdge* edge);
        size_t replaceBoundary(HalfEdge* edge, HalfEdge* with);
        size_t replaceBoundary(HalfEdge* from, HalfEdge* to, HalfEdge* with);
        void replaceEntireBoundary(HalfEdgeList& newBoundary);
        size_t countAndSetFace(HalfEdge* from, HalfEdge* until, Face* face);
        void updateBoundaryFaces(Face* face);
    };
public:
    struct GetVertexPosition {
        const V& operator()(const Vertex* vertex) const;
        const V& operator()(const HalfEdge* halfEdge) const;
    };

    class Callback {
    public:
        virtual ~Callback();
    public: // factory methods
        virtual Plane<T,3> plane(const Face* face) const;
        virtual void faceWasCreated(Face* face);
        virtual void faceWillBeDeleted(Face* face);
        virtual void faceDidChange(Face* face);
        virtual void faceWasSplit(Face* original, Face* clone);
        virtual void facesWillBeMerged(Face* remaining, Face* toDelete);
    };
protected:
    VertexList m_vertices;
    EdgeList m_edges;
    FaceList m_faces;
    BBox<T,3> m_bounds;
public: // Constructors
    Polyhedron();
    
    Polyhedron(const V& p1, const V& p2, const V& p3, const V& p4);
    template <typename C> Polyhedron(const V& p1, const V& p2, const V& p3, const V& p4, C& callback);

    Polyhedron(const BBox<T,3>& bounds);
    template <typename C> Polyhedron(const BBox<T,3>& bounds, C& callback);
    
    Polyhedron(typename V::List positions);
    template <typename C> Polyhedron(typename V::List positions, C& callback);

    Polyhedron(const Polyhedron<T,FP>& other);
private: // Constructor helpers
    template <typename C> void addPoints(const V& p1, const V& p2, const V& p3, const V& p4, C& callback);
    template <typename C> void setBounds(const BBox<T,3>& bounds, C& callback);
private: // Copy constructor
    class Copy;
public: // Destructor
    virtual ~Polyhedron();
public: // operators
    Polyhedron<T,FP>& operator=(Polyhedron<T,FP> other);
public: // swap function
    friend void swap(Polyhedron<T,FP>& first, Polyhedron<T,FP>& second) {
        using std::swap;
        swap(first.m_vertices, second.m_vertices);
        swap(first.m_edges, second.m_edges);
        swap(first.m_faces, second.m_faces);
    }
public: // Accessors
    size_t vertexCount() const;
    const VertexList& vertices() const;
    bool hasVertex(const V& position) const;
    
    size_t edgeCount() const;
    const EdgeList& edges() const;
    bool hasEdge(const V& pos1, const V& pos2) const;
    
    size_t faceCount() const;
    const FaceList& faces() const;
    bool hasFace(const typename V::List& positions) const;
    
    const BBox<T,3>& bounds() const;
    
    bool empty() const;
    bool point() const;
    bool edge() const;
    bool polygon() const;
    bool polyhedron() const;
    bool closed() const;

    void clear();
    
    struct FaceHit;
    FaceHit pickFace(const Ray<T,3>& ray) const;
private: // General purpose methods
    Vertex* findVertexByPosition(const V& position, T epsilon = Math::Constants<T>::almostZero()) const;
    Edge* findEdgeByPositions(const V& pos1, const V& pos2, T epsilon = Math::Constants<T>::almostZero()) const;
    Face* findFaceByPositions(const typename V::List& positions, T epsilon = Math::Constants<T>::almostZero()) const;
    
    bool checkInvariant() const;
    bool checkConvex() const;
    bool checkClosed() const;
    bool checkNoCoplanarFaces() const;
    bool checkNoDegenerateFaces() const;
    
    void updateBounds();
private:  // Moving vertices
    struct MoveVertexResult;
public:
    struct MoveVerticesResult {
        typename V::List movedVertices;
        typename V::List deletedVertices;
        typename V::List unchangedVertices;
        typename V::List newVertexPositions;
        typename V::List unknownVertices;
        
        MoveVerticesResult();
        MoveVerticesResult(const typename V::List& i_movedVertices);

        void add(const MoveVertexResult& result);
        void addUnknown(const V& position);
        bool allVerticesMoved() const;
        bool hasDeletedVertices() const;
        bool hasUnchangedVertices() const;
        bool hasUnknownVertices() const;
    };
    
    MoveVerticesResult moveVertices(const typename V::List& positions, const V& delta, bool allowMergeIncidentVertices);
    template <typename C> MoveVerticesResult moveVertices(typename V::List positions, const V& delta, bool allowMergeIncidentVertices, C& callback);
    
    MoveVerticesResult splitEdge(const V& v1, const V& v2, const V& delta);
    template <typename C> MoveVerticesResult splitEdge(const V& v1, const V& v2, const V& delta, C& callback);
    
    MoveVerticesResult splitFace(const typename V::List& vertexPositions, const V& delta);
    template <typename C> MoveVerticesResult splitFace(const typename V::List& vertexPositions, const V& delta, C& callback);
private: // Splitting edges and faces
    struct SplitResult;
    template <typename C> SplitResult splitEdge(const V& v1, const V& v2, C& callback);
    template <typename C> SplitResult splitFace(const typename V::List& vertexPositions, C& callback);
private:
    template <typename C> MoveVerticesResult doMoveVertices(typename V::List positions, const V& delta, bool allowMergeIncidentVertices, C& callback);

    template <typename C> MoveVertexResult moveVertex(Vertex* vertex, const V& destination, bool allowMergeIncidentVertex, C& callback);
    MoveVertexResult movePointVertex(Vertex* vertex, const V& destination);
    MoveVertexResult moveEdgeVertex(Vertex* vertex, const V& destination, bool allowMergeIncidentVertex);
    MoveVertexResult movePolygonVertex(Vertex* vertex, const V& destination, bool allowMergeIncidentVertex);
    template <typename C> MoveVertexResult movePolyhedronVertex(Vertex* vertex, const V& destination, bool allowMergeIncidentVertex, C& callback);

    template <typename C> void splitIncidentFaces(Vertex* vertex, const V& destination, C& callback);
    template <typename C> void chopFace(Face* face, HalfEdge* halfEdge, C& callback);
    template <typename C> void splitFace(Face* face, HalfEdge* halfEdge, C& callback);
    
    T computeNextMergePoint(Vertex* vertex, const V& origin, const V& destination, T lastFrac) const;
    T computeNextMergePointForIncidentNeighbour(HalfEdge* edge, const V& origin, const V& destination, T lastFrac) const;
    T computeNextMergePointForOppositeNeighbour(HalfEdge* edge, const V& origin, const V& destination, T lastFrac) const;
    T computeNextMergePointForPlane(const V& origin, const V& destination, const Plane<T,3>& plane, T lastFrac) const;
    
    template <typename C> void mergeVertices(HalfEdge* connectingEdge, C& callback);

    struct CleanupResult;
    template <typename C> CleanupResult cleanupAfterVertexMove(Vertex* vertex, C& callback);

    template <typename C> void mergeLeavingEdges(Vertex* vertex, C& callback);
    template <typename C> Edge* mergeIncomingAndLeavingEdges(Vertex* vertex, C& callback);
    template <typename C> void mergeNeighboursOfColinearEdges(HalfEdge* edge1, HalfEdge* edge2, C& callback);
    Edge* mergeColinearEdges(HalfEdge* edge1, HalfEdge* edge2);

    template <typename C> Face* mergeIncidentFaces(Vertex* vertex, C& callback);
    template <typename C> void mergeNeighbours(HalfEdge* borderFirst, C& callback);
    
    template <typename C> void incidentFacesDidChange(Vertex* vertex, C& callback);
public: // Convex hull and adding points
    template <typename I> void addPoints(I cur, I end);
    template <typename I, typename C> void addPoints(I cur, I end, C& callback);
    void addPoint(const V& position);
    template <typename C> void addPoint(const V& position, C& callback);
private:
    class Seam;

    void addFirstPoint(const V& position);
    void addSecondPoint(const V& position);
    
    template <typename C> void addThirdPoint(const V& position, C& callback);
    void addPointToEdge(const V& position);
    
    template <typename C> void addFurtherPoint(const V& position, C& callback);
    template <typename C> void addFurtherPointToPolygon(const V& position, C& callback);
    template <typename C> void addPointToPolygon(const V& position, C& callback);
    template <typename C> void makePolygon(const typename V::List& positions, C& callback);
    template <typename C> void makePolyhedron(const V& position, C& callback);
    
    template <typename C> void addFurtherPointToPolyhedron(const V& position, C& callback);
    template <typename C> void addPointToPolyhedron(const V& position, const Seam& seam, C& callback);
    
    class SplittingCriterion;
    class SplitByVisibilityCriterion;
    class SplitByNormalCriterion;
    
    Seam createSeam(const SplittingCriterion& criterion);
    
    typedef std::set<Face*> FaceSet;
    template <typename C> void split(const Seam& seam, C& callback);
    template <typename C> void deleteFaces(HalfEdge* current, FaceSet& visitedFaces, VertexList& verticesToDelete, C& callback);
    
    template <typename C> void weaveCap(const Seam& seam, C& callback);
    template <typename C> Vertex* weaveCap(const Seam& seam, const V& position, C& callback);
    template <typename C> Face* createCapTriangle(HalfEdge* h1, HalfEdge* h2, HalfEdge* h3, C& callback) const;
public: // Clipping
    struct ClipResult {
        typedef enum {
            Type_ClipUnchanged,
            Type_ClipEmpty,
            Type_ClipSuccess
        } Type;
        
        const Type type;
        ClipResult(const Type i_type);
        bool unchanged() const;
        bool empty() const;
        bool success() const;
    };

    ClipResult clip(const Plane<T,3>& plane);
    template <typename C> ClipResult clip(const Plane<T,3>& plane, C& callback);
private:
    template <typename C> bool isCoplanarToAnyFace(const Plane<T,3>& plane, const C& callback) const;
    ClipResult checkIntersects(const Plane<T,3>& plane) const;

    template <typename C> Seam intersectWithPlane(const Plane<T,3>& plane, C& callback);
    HalfEdge* findInitialIntersectingEdge(const Plane<T,3>& plane) const;
    template <typename C> HalfEdge* intersectWithPlane(HalfEdge* firstBoundaryEdge, const Plane<T,3>& plane, C& callback);
    template <typename C> void intersectWithPlane(HalfEdge* remainingFirst, HalfEdge* deletedFirst, C& callback);
    HalfEdge* findNextIntersectingEdge(HalfEdge* searchFrom, const Plane<T,3>& plane) const;
};

#endif
