#ifndef MXVDPMSLIM_INCLUDED // -*- C++ -*-
#define MXVDPMSLIM_INCLUDED
#if !defined(__GNUC__)
#  pragma once
#endif

#include "MxStdSlim.h"
#include "MxQMetric.h"
#include "MxGeom3D.h"

class MxVdpmVector : public MxVector
{
public:
    MxVdpmVector() : MxVector(11) { }
};

class MxVdpmSlim;

class MxVdpmPairContraction : public MxPairContraction
{
private:
    void compute_radius_from_child(const MxVdpmPairContraction&);
    void compute_radius_from_vertex(MxVertexID, const MxBlock<float>&, MxStdModel*);

    float compute_cone_from_child(const MxDynBlock<MxVdpmPairContraction>&, const MxVdpmPairContraction&, MxVdpmSlim&, float[3]);
    float compute_cone_from_vertex(MxVertexID, MxVdpmSlim&, float[3]);

    void collect_neighbors_from_child(const MxDynBlock<MxVdpmPairContraction>&, const MxVdpmPairContraction&, MxVdpmSlim&, MxFaceList&);
    void collect_neighbors_from_vertex(MxVertexID, MxVdpmSlim&, MxFaceList&);

public:
    uint index, parent, child_v1, child_v2, v_i;
    MxVdpmVector vt, vu, vs;
    float radius;
    float sin2alpha;
    float uni_error;
    float dir_error;
    FID fl, fr, fn0, fn1, fn2, fn3;

    MxVdpmPairContraction() : index(UINT_MAX), parent(UINT_MAX), child_v1(UINT_MAX), child_v2(UINT_MAX), v_i(UINT_MAX),
        radius(0.0f), sin2alpha(0.0f), uni_error(0.0f), dir_error(0.0f),
        fn0(UINT_MAX), fn1(UINT_MAX), fn2(UINT_MAX), fn3(UINT_MAX) { }

    void update_radius(const MxDynBlock<MxVdpmPairContraction>&, const MxBlock<float>&, MxStdModel*);
    void update_cone(const MxDynBlock<MxVdpmPairContraction>&, MxVdpmSlim&);
    void update_deviation(const MxDynBlock<MxVdpmPairContraction>&, MxVdpmSlim&);
    bool update_faces(MxVdpmSlim&);
};

class MxVdpmSlim : public MxStdSlim
{
private:
    uint D;

    class edge_info : public MxHeapable
    {
    public:
        MxVertexID v1, v2;
        MxVector target;

        MxVertexID opposite_vertex(MxVertexID v)
        {
            if (v == v1) return v2;
            else { SanityCheck(v == v2); return v1; }
        }

        edge_info(uint D) : target(D) { }
    };
    typedef MxSizedDynBlock<edge_info*, 6> edge_list;

    MxBlock<edge_list> edge_links;	// 1 per vertex
    MxBlock<MxQuadric*> __quadrics;	// 1 per vertex

    //
    // Temporary variables used by methods
    MxVertexList star, star2;

protected:
    uint compute_dimension(MxStdModel *);
    void pack_to_vector(MxVertexID, MxVector&);
    void unpack_from_vector(MxVertexID, MxVector&);
    uint prop_count();
    void pack_prop_to_vector(MxVertexID, MxVector&, uint);
    void unpack_prop_from_vector(MxVertexID, MxVector&, uint);

    void compute_face_quadric(MxFaceID, MxQuadric&);
    void collect_quadrics();

    void create_edge(MxVertexID, MxVertexID);
    void collect_edges();
    void constrain_boundaries();
    void discontinuity_constraint(MxVertexID, MxVertexID, const MxFaceList&);
    void compute_edge_info(edge_info *);
    void finalize_edge_update(edge_info *);
    void compute_target_placement(edge_info *);

    void apply_contraction(const MxPairContraction&, edge_info *);
    void update_pre_contract(const MxPairContraction&);
    void update_pre_expand(const MxPairContraction&);
    void update_post_expand(const MxPairContraction&);

    void collect_radiuses();
    void collect_cones();
    void collect_geoms();

public:
    bool use_color;
    bool use_texture;
    bool use_normals;
    bool will_decouple_quadrics;

    MxBlock<float[3]> vertex_geoms;			// 1 per vertex
    MxBlock<float> vertex_radiuses;			// 1 per vertex
    MxBlock<float[3]> face_normals;			// 1 per face
    MxBlock<MxFaceList> vertex_neighbors;	// 1 per vertex
    MxBounds bounds;

public:
    MxVdpmSlim(MxStdModel *);
    virtual ~MxVdpmSlim();

    uint dim() const { return D; }

    void consider_color(bool will = true);
    void consider_texture(bool will = true);
    void consider_normals(bool will = true);

    uint quadric_count() const { return __quadrics.length(); }
    MxQuadric&       quadric(uint i)       { return *(__quadrics(i)); }
    const MxQuadric& quadric(uint i) const { return *(__quadrics(i)); }


    void initialize();
    bool decimate(uint);
    bool check_model();

    void apply_expansion(const MxPairContraction& conx);
    const MxQuadric& vertex_quadric(MxVertexID v) { return quadric(v); }
    void(*contraction_callback)(const MxPairContraction&, float);
};

// MXVDPMSLIM_INCLUDED
#endif
