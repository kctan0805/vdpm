#include "stdmix.h"
#include "MxVdpmSlim.h"
#include "MxGeom3D.h"

#include "compute_error.h"
#include "geomutils.h"
#include "block_list.h"

typedef MxQuadric Quadric;

MxVdpmSlim::MxVdpmSlim(MxStdModel *m0)
    : MxStdSlim(m0),
    __quadrics(m0->vert_count()),
    edge_links(m0->vert_count()),
    vertex_geoms(m0->vert_count()),
    vertex_radiuses(m0->vert_count()),
    face_normals(m0->face_count()),
    vertex_neighbors(m0->vert_count())
{
    consider_color();
    consider_texture();

    if (m->normal_binding() == MX_UNBOUND)
    {
        m->normal_binding(MX_PERVERTEX);
        m->synthesize_normals();
    }
    consider_normals();

    D = compute_dimension(m);

    bounds.reset();

    for (MxVertexID v = 0; v<m->vert_count(); v++)
        bounds.add_point(m->vertex(v));

    bounds.complete();

    will_decouple_quadrics = false;
    contraction_callback = NULL;
}

MxVdpmSlim::~MxVdpmSlim()
{
    uint i;

    // Delete everything remaining in the heap
    for (i = 0; i < heap.size(); i++)
        delete ((edge_info *)heap.item(i));

    for (i = 0; i < quadric_count(); i++)
        delete __quadrics[i];
}

void MxVdpmSlim::consider_color(bool will)
{
    use_color = will && (m->color_binding() == MX_PERVERTEX);
    D = compute_dimension(m);
}

void MxVdpmSlim::consider_texture(bool will)
{
    use_texture = will && (m->texcoord_binding() == MX_PERVERTEX);
    D = compute_dimension(m);
}

void MxVdpmSlim::consider_normals(bool will)
{
    use_normals = will && (m->normal_binding() == MX_PERVERTEX);
    D = compute_dimension(m);
}

uint MxVdpmSlim::compute_dimension(MxStdModel *m)
{
    uint d = 3;

    if (use_color)  d += 3;
    if (use_texture)  d += 2;
    if (use_normals)  d += 3;

    return d;
}

void MxVdpmSlim::pack_to_vector(MxVertexID id, MxVector& v)
{
    SanityCheck(v.dim() >= D);
    SanityCheck(id < m->vert_count());

    v[0] = m->vertex(id)[0];
    v[1] = m->vertex(id)[1];
    v[2] = m->vertex(id)[2];

    uint i = 3;
    if (use_color)
    {
        v[i++] = m->color(id).R();
        v[i++] = m->color(id).G();
        v[i++] = m->color(id).B();
    }
    if (use_texture)
    {
        v[i++] = m->texcoord(id)[0];
        v[i++] = m->texcoord(id)[1];
    }
    if (use_normals)
    {
        v[i++] = m->normal(id)[0];
        v[i++] = m->normal(id)[1];
        v[i++] = m->normal(id)[2];
    }
}

void MxVdpmSlim::pack_prop_to_vector(MxVertexID id, MxVector& v, uint target)
{
    if (target == 0)
    {
        v[0] = m->vertex(id)[0];
        v[1] = m->vertex(id)[1];
        v[2] = m->vertex(id)[2];
        return;
    }

    uint i = 3;
    target--;

    if (use_color)
    {
        if (target == 0)
        {
            v[i] = m->color(id).R();
            v[i + 1] = m->color(id).G();
            v[i + 2] = m->color(id).B();
            return;
        }
        i += 3;
        target--;
    }
    if (use_texture)
    {
        if (target == 0)
        {
            v[i] = m->texcoord(id)[0];
            v[i + 1] = m->texcoord(id)[1];
            return;
        }
        i += 2;
        target--;
    }
    if (use_normals)
    {
        if (target == 0)
        {
            v[i] = m->normal(id)[0];
            v[i + 1] = m->normal(id)[1];
            v[i + 2] = m->normal(id)[2];
            return;
        }
    }
}

static inline void CLAMP(double& v, double lo, double hi)
{
    if (v < lo) v = lo;
    if (v > hi) v = hi;
}

void MxVdpmSlim::unpack_from_vector(MxVertexID id, MxVector& v)
{
    SanityCheck(v.dim() >= D);
    SanityCheck(id < m->vert_count());

    m->vertex(id)[0] = v[0];
    m->vertex(id)[1] = v[1];
    m->vertex(id)[2] = v[2];

    uint i = 3;
    if (use_color)
    {
        CLAMP(v[i], 0, 1);
        CLAMP(v[i + 1], 0, 1);
        CLAMP(v[i + 2], 0, 1);
        m->color(id).set(v[i], v[i + 1], v[i + 2]);
        i += 3;
    }
    if (use_texture)
    {
        m->texcoord(id)[0] = v[i++];
        m->texcoord(id)[1] = v[i++];
    }
    if (use_normals)
    {
        float n[3];  n[0] = v[i++];  n[1] = v[i++];  n[2] = v[i++];
        mxv_unitize(n, 3);
        m->normal(id).set(n[0], n[1], n[2]);
    }
}

void MxVdpmSlim::unpack_prop_from_vector(MxVertexID id, MxVector& v, uint target)
{
    if (target == 0)
    {
        m->vertex(id)[0] = v[0];
        m->vertex(id)[1] = v[1];
        m->vertex(id)[2] = v[2];
        return;
    }

    uint i = 3;
    target--;

    if (use_color)
    {
        if (target == 0)
        {
            m->color(id).set(v[i], v[i + 1], v[i + 2]);
            return;
        }
        i += 3;
        target--;
    }
    if (use_texture)
    {
        if (target == 0)
        {
            m->texcoord(id)[0] = v[i];
            m->texcoord(id)[1] = v[i + 1];
            return;
        }
        i += 2;
        target--;
    }
    if (use_normals)
    {
        if (target == 0)
        {
            float n[3];  n[0] = v[i];  n[1] = v[i + 1];  n[2] = v[i + 2];
            mxv_unitize(n, 3);
            m->normal(id).set(n[0], n[1], n[2]);
            return;
        }
    }
}


uint MxVdpmSlim::prop_count()
{
    uint i = 1;

    if (use_color) i++;
    if (use_texture) i++;
    if (use_normals) i++;

    return i;
}

void MxVdpmSlim::compute_face_quadric(MxFaceID i, MxQuadric& Q)
{
    MxFace& f = m->face(i);

    MxVector v1(dim());
    MxVector v2(dim());
    MxVector v3(dim());

    if (will_decouple_quadrics)
    {
        Q.clear();

        for (uint p = 0; p < prop_count(); p++)
        {
            v1 = 0.0;  v2 = 0.0;  v3 = 0.0;

            pack_prop_to_vector(f[0], v1, p);
            pack_prop_to_vector(f[1], v2, p);
            pack_prop_to_vector(f[2], v3, p);

            // !!BUG: Will count area multiple times (once per property)
            MxQuadric Q_p(v1, v2, v3, m->compute_face_area(i));

            // !!BUG: Need to only extract the relevant block of the matrix.
            //        Adding the whole thing gives us extraneous stuff.
            Q += Q_p;
        }
    }
    else
    {
        pack_to_vector(f[0], v1);
        pack_to_vector(f[1], v2);
        pack_to_vector(f[2], v3);

        Q = MxQuadric(v1, v2, v3, m->compute_face_area(i));
    }
}

void MxVdpmSlim::collect_quadrics()
{
    for (uint j = 0; j < quadric_count(); j++)
        __quadrics[j] = new MxQuadric(dim());

    for (MxFaceID i = 0; i < m->face_count(); i++)
    {
        MxFace& f = m->face(i);

        MxQuadric Q(dim());
        compute_face_quadric(i, Q);

        // 	if( weight_by_area )
        // 	    Q *= Q.area();

        quadric(f[0]) += Q;
        quadric(f[1]) += Q;
        quadric(f[2]) += Q;
    }
}

void MxVdpmSlim::initialize()
{
    collect_quadrics();

    if (boundary_weight > 0.0)
        constrain_boundaries();

    collect_edges();
    collect_radiuses();
    collect_cones();
    collect_geoms();

    is_initialized = true;
}

void MxVdpmSlim::compute_target_placement(edge_info *info)
{
    MxVertexID i = info->v1, j = info->v2;

    const MxQuadric &Qi = quadric(i), &Qj = quadric(j);
    MxQuadric Q = Qi;  Q += Qj;

    double err;

    if (Q.optimize(info->target))
    {
        err = Q(info->target);
    }
    else
    {
        // Fall back only on endpoints

        MxVector v_i(dim()), v_j(dim());

        pack_to_vector(i, v_i);
        pack_to_vector(j, v_j);

        double e_i = Q(v_i);
        double e_j = Q(v_j);

        if (e_i <= e_j)
        {
            info->target = v_i;
            err = e_i;
        }
        else
        {
            info->target = v_j;
            err = e_j;
        }
    }

    if (err < 0) err += 1e200; // The punishment of negative error, make it a great positive error
    err += 1e-10 * Q.area();  // Make sure that area works even the error is zero

    //     if( weight_by_area )
    // 	err / Q.area();
    info->heap_key(-err);
}

bool MxVdpmSlim::decimate(uint target)
{
    MxVdpmPairContraction conx;

#if (SAFETY >= 2)
    for (unsigned int i = 0; i < m->vert_count(); ++i)
    {
        if (m->vertex_is_valid(i))
        {
            mxmsg_signalf(MXMSG_DEBUG, "v[%u] p:{%f %f %f}",
                i, m->vertex(i)[0], m->vertex(i)[1], m->vertex(i)[2]);
        }
    }
    for (unsigned int i = 0; i < m->face_count(); ++i)
    {
        if (m->face_is_valid(i))
            mxmsg_signalf(MXMSG_DEBUG, "f[%u]{%u %u %u}", i, m->face(i)(0), m->face(i)(1), m->face(i)(2));
    }
#endif

    while (valid_faces > target)
    {
        edge_info *info = (edge_info *)heap.extract();
        if (!info)
            return false;

        MxVertexID v1 = info->v1, v2 = info->v2;

        if (m->vertex_is_valid(v1) && m->vertex_is_valid(v2))
        {
            m->compute_contraction(v1, v2, &conx);

            conx.dv1[X] = (float)info->target[X] - m->vertex(v1)[X];
            conx.dv1[Y] = (float)info->target[Y] - m->vertex(v1)[Y];
            conx.dv1[Z] = (float)info->target[Z] - m->vertex(v1)[Z];
            conx.dv2[X] = (float)info->target[X] - m->vertex(v2)[X];
            conx.dv2[Y] = (float)info->target[Y] - m->vertex(v2)[Y];
            conx.dv2[Z] = (float)info->target[Z] - m->vertex(v2)[Z];

            pack_to_vector(v1, conx.vt);
            pack_to_vector(v2, conx.vu);

            if (!conx.update_faces(*this))
            {
                uint i;

                for (i = 0; i < edge_links(v1).size(); i++)
                    if (edge_links(v1)(i) == info)
                    {
                        edge_links(v1).remove(i);
                        break;
                    }

                for (i = 0; i < edge_links(v2).size(); i++)
                    if (edge_links(v2)(i) == info)
                    {
                        edge_links(v2).remove(i);
                        break;
                    }

                delete info;
                continue;
            }
            apply_contraction(conx, info);

            pack_to_vector(v1, conx.vs);

            if (contraction_callback)
                (*contraction_callback)(conx, -info->heap_key());
        }
        delete info;

        //if (valid_faces < m->face_count() / 10 && !check_model())
        //    return false;
    }
    return true;
}



////////////////////////////////////////////////////////////////////////
//
// This is *very* close to the code in MxEdgeQSlim

void MxVdpmSlim::create_edge(MxVertexID i, MxVertexID j)
{
    edge_info *info = new edge_info(dim());

    edge_links(i).add(info);
    edge_links(j).add(info);

    info->v1 = i;
    info->v2 = j;

    compute_edge_info(info);
}

void MxVdpmSlim::discontinuity_constraint(MxVertexID i, MxVertexID j,
    const MxFaceList& faces)
{
    for (uint f = 0; f < faces.length(); f++)
    {
        Vec3 org(m->vertex(i)), dest(m->vertex(j));
        Vec3 e = dest - org;

        Vec3 n;
        m->compute_face_normal(faces[f], n);

        Vec3 n2 = e ^ n;
        unitize(n2);

        MxQuadric3 Q3(n2, -(n2*org));
        Q3 *= boundary_weight;

        MxQuadric Q(Q3, dim());

        quadric(i) += Q;
        quadric(j) += Q;
    }
}

void MxVdpmSlim::apply_contraction(const MxPairContraction& conx,
    edge_info *info)
{
    valid_verts--;
    valid_faces -= conx.dead_faces.length();
    quadric(conx.v1) += quadric(conx.v2);

    update_pre_contract(conx);

    m->apply_contraction(conx);

    unpack_from_vector(conx.v1, info->target);

    // Must update edge_info here so that the meshing penalties
    // will be computed with respect to the new mesh rather than the old
    for (uint i = 0; i < edge_links(conx.v1).length(); i++)
        compute_edge_info(edge_links(conx.v1)[i]);
}



////////////////////////////////////////////////////////////////////////
//
// These were copied *unmodified* from MxEdgeQSlim
// (with some unsupported features commented out).
//

void MxVdpmSlim::collect_edges()
{
    MxVertexList star;
    MxFaceList faces;

    for (MxVertexID i = 0; i < m->vert_count(); i++)
    {
        star.reset();
        m->collect_vertex_star(i, star);

        for (uint j = 0; j < star.length(); j++)
        {
            if (i < star(j))  // Only add particular edge once
            {
                faces.reset();
                m->collect_edge_neighbors(i, star[j], faces);
                if (faces.length() > 0 && faces.length() <= 2)
                {
                    create_edge(i, star(j));
                }
                else if (faces.length() > 2)
                    mxmsg_signal(MXMSG_NOTE, "Ignoring non-manifold edge");
                // ELSE: ignore boundary edges
            }
        }
    }
}

void MxVdpmSlim::constrain_boundaries()
{
    MxVertexList star;
    MxFaceList faces;

    for (MxVertexID i = 0; i < m->vert_count(); i++)
    {
        star.reset();
        m->collect_vertex_star(i, star);

        for (uint j = 0; j < star.length(); j++)
            if (i < star(j))
            {
                faces.reset();
                m->collect_edge_neighbors(i, star(j), faces);
                if (faces.length() == 1)
                    discontinuity_constraint(i, star(j), faces);
            }
    }
}

void MxVdpmSlim::compute_edge_info(edge_info *info)
{
    compute_target_placement(info);

    //     if( will_normalize_error )
    //     {
    //         double e_max = Q_max(info->vnew);
    //         if( weight_by_area )
    //             e_max /= Q_max.area();

    //         info->heap_key(info->heap_key() / e_max);
    //     }

    finalize_edge_update(info);
}

void MxVdpmSlim::finalize_edge_update(edge_info *info)
{
    //     if( meshing_penalty > 1.0 )
    //         apply_mesh_penalties(info);

    if (info->is_in_heap())
        heap.update(info);
    else
        heap.insert(info);
#if (SAFETY >= 2)
    mxmsg_signalf(MXMSG_TRACE, "info[%d] {%u %u}", info->get_heap_pos(), info->v1, info->v2);
#endif
}

void MxVdpmSlim::update_pre_contract(const MxPairContraction& conx)
{
    MxVertexID v1 = conx.v1, v2 = conx.v2;
    uint i, j;

    star.reset();
    //
    // Before, I was gathering the vertex "star" using:
    //      m->collect_vertex_star(v1, star);
    // This doesn't work when we initially begin with a subset of
    // the total edges.  Instead, we need to collect the "star"
    // from the edge links maintained at v1.
    //
    for (i = 0; i < edge_links(v1).length(); i++)
        star.add(edge_links(v1)[i]->opposite_vertex(v1));

    for (i = 0; i < edge_links(v2).length(); i++)
    {
        edge_info *e = edge_links(v2)(i);
        MxVertexID u = (e->v1 == v2) ? e->v2 : e->v1;
        SanityCheck(e->v1 == v2 || e->v2 == v2);
        SanityCheck(u != v2);

        if (u == v1 || varray_find(star, u))
        {
            // This is a useless link --- kill it
            bool found = varray_find(edge_links(u), e, &j);
            assert(found);
            edge_links(u).remove(j);
            heap.remove(e);
            if (u != v1) delete e; // (v1,v2) will be deleted later
        }
        else
        {
            // Relink this to v1
            e->v1 = v1;
            e->v2 = u;
            edge_links(v1).add(e);
        }
    }

    edge_links(v2).reset();
}

void MxVdpmSlim::update_pre_expand(const MxPairContraction&)
{
}

void MxVdpmSlim::update_post_expand(const MxPairContraction& conx)
{
    MxVertexID v1 = conx.v1, v2 = conx.v2;
    uint i;

    star.reset(); star2.reset();
    PRECAUTION(edge_links(conx.v2).reset());
    m->collect_vertex_star(conx.v1, star);
    m->collect_vertex_star(conx.v2, star2);

    i = 0;
    while (i < edge_links(v1).length())
    {
        edge_info *e = edge_links(v1)(i);
        MxVertexID u = (e->v1 == v1) ? e->v2 : e->v1;
        SanityCheck(e->v1 == v1 || e->v2 == v1);
        SanityCheck(u != v1 && u != v2);

        bool v1_linked = varray_find(star, u);
        bool v2_linked = varray_find(star2, u);

        if (v1_linked)
        {
            if (v2_linked)  create_edge(v2, u);
            i++;
        }
        else
        {
            // !! BUG: I expected this to be true, but it's not.
            //         Need to find out why, and whether it's my
            //         expectation or the code that's wrong.
            // SanityCheck(v2_linked);
            e->v1 = v2;
            e->v2 = u;
            edge_links(v2).add(e);
            edge_links(v1).remove(i);
        }

        compute_edge_info(e);
    }

    if (varray_find(star, v2))
        // ?? BUG: Is it legitimate for there not to be an edge here ??
        create_edge(v1, v2);
}

void MxVdpmSlim::apply_expansion(const MxPairContraction& conx)
{
    update_pre_expand(conx);

    m->apply_expansion(conx);

    //
    // Post-expansion update
    valid_verts++;
    valid_faces += conx.dead_faces.length();
    quadric(conx.v1) -= quadric(conx.v2);

    update_post_expand(conx);

    MxVdpmPairContraction &prop_conx = (MxVdpmPairContraction &)conx;
    unpack_from_vector(conx.v1, prop_conx.vt);
    unpack_from_vector(conx.v2, prop_conx.vu);
}

void MxVdpmSlim::collect_radiuses()
{
    for (MxVertexID v = 0; v < m->vert_count(); v++)
    {
        const MxFaceList& faces = m->neighbors(v);

        vertex_radiuses[v] = 0.0f;

        for (int i = 0; i < faces.length(); i++)
            for (uint j = 0; j < 3; j++)
            {
                MxVertexID id = m->face(faces(i))(j);
                if (id != v)
                {
                    float dv[3];

                    mxv_sub(dv, m->vertex(id), m->vertex(v), 3);

                    float radius = mxv_norm(dv, 3);
                    if (vertex_radiuses[v] < radius)
                        vertex_radiuses[v] = radius;
                }
            }
    }
}

void MxVdpmSlim::collect_cones()
{
    for (MxFaceID f = 0; f < m->face_count(); f++)
    {
        float n[3];

        m->compute_face_normal(f, n);

        for (uint i = 0; i < 3; i++)
            face_normals(f)[i] = n[i];
    }

    for (MxVertexID v = 0; v < m->vert_count(); v++)
    {
        vertex_neighbors(v).resize(m->neighbors(v).length());
        vertex_neighbors(v).copy(m->neighbors(v));
    }
}

void MxVdpmSlim::collect_geoms()
{
    for (MxVertexID v = 0; v < m->vert_count(); v++)
    {
        vertex_geoms(v)[0] = m->vertex(v)[0];
        vertex_geoms(v)[1] = m->vertex(v)[1];
        vertex_geoms(v)[2] = m->vertex(v)[2];
    }
}

static struct model* create_model(MxVdpmSlim& slim)
{
    struct model *tmesh;
    vertex_t bbmin, bbmax;
    int nvtcs = 0;
    int nfaces = 0;
    int f0, f1, f2;
    float x, y, z;
    int rcode = 1;
    struct block_list *head_verts, *cur_vert;
    struct block_list *head_faces, *cur_face;

    head_verts = (struct block_list*)malloc(sizeof(struct block_list));
    head_faces = (struct block_list*)malloc(sizeof(struct block_list));

    cur_vert = head_verts;
    cur_face = head_faces;

    bbmin.x = bbmin.y = bbmin.z = FLT_MAX;
    bbmax.x = bbmax.y = bbmax.z = -FLT_MAX;
    tmesh = (struct model*)calloc(1, sizeof(struct model));

    rcode = init_block_list(head_verts, sizeof(vertex_t));
    if (rcode < 0)
        return NULL;

    rcode = init_block_list(head_faces, sizeof(face_t));
    if (rcode < 0)
        return NULL;

    // Collects vertex list
    MxVertexList vertices;
    int i;

    vertices.reset();
    for (i = 0; i < slim.model().vert_count(); i++)
    {
        if (slim.model().vertex_is_valid(i))
        {
            vertices.add(i);
        }
    }

    MxBlock<MxVertexID> vmap(slim.model().vert_count());  // Maps old VIDs to new VIDs
    MxVertexID next_vert = 0;

    for (i = 0; i < vertices.length(); i++)
    {
        vmap(vertices(i)) = next_vert++;

        MxVertex& mv = slim.model().vertex(vertices(i));

        x = mv(X);
        y = mv(Y);
        z = mv(Z);

        if (cur_vert->elem_filled == cur_vert->nelem) {
            /* Reallocate storage if needed */
            cur_vert = get_next_block(cur_vert);
            if (cur_vert == NULL) {
                return NULL;
            }
        }
        BLOCK_LIST_TAIL(cur_vert, vertex_t).x = x;
        BLOCK_LIST_TAIL(cur_vert, vertex_t).y = y;
        BLOCK_LIST_TAIL_INCR(cur_vert, vertex_t).z = z;
        nvtcs++;

        if (x < bbmin.x) bbmin.x = x;
        if (x > bbmax.x) bbmax.x = x;
        if (y < bbmin.y) bbmin.y = y;
        if (y > bbmax.y) bbmax.y = y;
        if (z < bbmin.z) bbmin.z = z;
        if (z > bbmax.z) bbmax.z = z;
    }

    for (i = 0; i < slim.model().face_count(); i++)
    {
        if (slim.model().face_is_valid(i))
        {
            MxFace& f = slim.model().face(i);

            f0 = vmap(f(0));
            f1 = vmap(f(1));
            f2 = vmap(f(2));

            if (cur_face->elem_filled == cur_face->nelem) {
                /* Reallocate storage if needed */
                cur_face = get_next_block(cur_face);
                if (cur_face == NULL) {
                    return NULL;
                }
            }

            BLOCK_LIST_TAIL(cur_face, face_t).f0 = f0;
            BLOCK_LIST_TAIL(cur_face, face_t).f1 = f1;
            BLOCK_LIST_TAIL_INCR(cur_face, face_t).f2 = f2;
            nfaces++;
        }
    }

    tmesh->bBox[0] = bbmin;
    tmesh->bBox[1] = bbmax;
    tmesh->num_vert = nvtcs;
    tmesh->num_faces = nfaces;
    tmesh->vertices = (vertex_t*)malloc(nvtcs*sizeof(vertex_t));
    tmesh->faces = (face_t*)malloc(nfaces*sizeof(face_t));
    rcode = gather_block_list(head_verts, tmesh->vertices,
        nvtcs*sizeof(vertex_t));
    rcode = gather_block_list(head_faces, tmesh->faces,
        nfaces*sizeof(face_t));
    free_block_list(&head_verts);
    free_block_list(&head_faces);

    return tmesh;
}

bool MxVdpmSlim::check_model()
{
    struct model_error model;
    struct model_info minfo;

    memset(&model, 0, sizeof(model));

    model.mesh = create_model(*this);
    analyze_model(model.mesh, &minfo, 0, 0, NULL, NULL);

    __free_raw_model(model.mesh);
    free_face_error(model.fe);

    if (!minfo.manifold || minfo.n_degenerate > 0 || minfo.n_disjoint_parts > 1)
        return false;

    return true;
}

void MxVdpmPairContraction::update_radius(const MxDynBlock<MxVdpmPairContraction>& history,
    const MxBlock<float>& vertex_radiuses, MxStdModel *m)
{
    if (child_v1 != UINT_MAX)
        compute_radius_from_child(history(child_v1));
    else
        compute_radius_from_vertex(v1, vertex_radiuses, m);

    if (child_v2 != UINT_MAX)
        compute_radius_from_child(history(child_v2));
    else
        compute_radius_from_vertex(v2, vertex_radiuses, m);
}

void MxVdpmPairContraction::compute_radius_from_child(const MxVdpmPairContraction& child)
{
    float dv[3], vs1[3], vs2[3];

    vs1[0] = (float)child.vs[0];
    vs1[1] = (float)child.vs[1];
    vs1[2] = (float)child.vs[2];
    vs2[0] = (float)vs[0];
    vs2[1] = (float)vs[1];
    vs2[2] = (float)vs[2];

    mxv_sub(dv, vs1, vs2, 3);

    float r = mxv_norm(dv, 3) + child.radius;
    if (radius < r)
        radius = r;
}

void MxVdpmPairContraction::compute_radius_from_vertex(MxVertexID v, const MxBlock<float>& vertex_radiuses, MxStdModel *m)
{
    float dv[3], vs1[3], vs2[3];

    vs1[0] = m->vertex(v)[0];
    vs1[1] = m->vertex(v)[1];
    vs1[2] = m->vertex(v)[2];
    vs2[0] = (float)vs[0];
    vs2[1] = (float)vs[1];
    vs2[2] = (float)vs[2];

    mxv_sub(dv, vs1, vs2, 3);

    float r = mxv_norm(dv, 3) + vertex_radiuses(v);
    if (radius < r)
        radius = r;
}

void MxVdpmPairContraction::update_cone(const MxDynBlock<MxVdpmPairContraction>& history, MxVdpmSlim& slim)
{
    float nv[3], dot, min_dot, alpha;
    uint i = 3;

    if (slim.use_color)
        i += 3;
    if (slim.use_texture)
        i += 2;

    nv[0] = (float)vs[i++];  nv[1] = (float)vs[i++];  nv[2] = (float)vs[i++];
    mxv_unitize(nv, 3);

    if (child_v1 != UINT_MAX)
        dot = compute_cone_from_child(history, history(child_v1), slim, nv);
    else
        dot = compute_cone_from_vertex(v1, slim, nv);

    min_dot = dot;

    if (child_v2 != UINT_MAX)
        dot = compute_cone_from_child(history, history(child_v2), slim, nv);
    else
        dot = compute_cone_from_vertex(v2, slim, nv);

    if (min_dot > dot)
        min_dot = dot;

    alpha = acos(min_dot);

    if (alpha > M_PI / 2.0f)
        alpha = (float)M_PI / 2.0f;

    sin2alpha = sinf(alpha);
    sin2alpha *= 2;
}

float MxVdpmPairContraction::compute_cone_from_child(const MxDynBlock<MxVdpmPairContraction>& history,
    const MxVdpmPairContraction& conx, MxVdpmSlim& slim, float nv[3])
{
    float min_dot, dot;

    if (conx.child_v1 != UINT_MAX)
        dot = compute_cone_from_child(history, history(conx.child_v1), slim, nv);
    else
        dot = compute_cone_from_vertex(v1, slim, nv);

    min_dot = dot;

    if (conx.child_v2 != UINT_MAX)
        dot = compute_cone_from_child(history, history(conx.child_v2), slim, nv);
    else
        dot = compute_cone_from_vertex(v2, slim, nv);

    if (min_dot > dot)
        min_dot = dot;

    return min_dot;
}

float MxVdpmPairContraction::compute_cone_from_vertex(MxVertexID v, MxVdpmSlim& slim, float nv[3])
{
    const MxFaceList& faces = slim.vertex_neighbors(v);
    float dot, min_dot = FLT_MAX;

    for (int i = 0; i < faces.length(); i++)
    {
        dot = fabs(mxv_dot(nv, slim.face_normals(i), 3));

        if (min_dot > dot)
            min_dot = dot;
    }
    return min_dot;
}

static struct model* create_model_from_simplified(MxVdpmSlim& slim, MxFaceList& faces)
{
    struct model *tmesh;
    vertex_t bbmin, bbmax;
    int nvtcs = 0;
    int nfaces = 0;
    int f0, f1, f2;
    float x, y, z;
    int rcode = 1;
    struct block_list *head_verts, *cur_vert;
    struct block_list *head_faces, *cur_face;

    head_verts = (struct block_list*)malloc(sizeof(struct block_list));
    head_faces = (struct block_list*)malloc(sizeof(struct block_list));

    cur_vert = head_verts;
    cur_face = head_faces;

    bbmin.x = bbmin.y = bbmin.z = FLT_MAX;
    bbmax.x = bbmax.y = bbmax.z = -FLT_MAX;
    tmesh = (struct model*)calloc(1, sizeof(struct model));

    rcode = init_block_list(head_verts, sizeof(vertex_t));
    if (rcode < 0)
        return NULL;

    rcode = init_block_list(head_faces, sizeof(face_t));
    if (rcode < 0)
        return NULL;

    // Collects vertex list
    MxVertexList vertices;
    int i, j, k;

    vertices.reset();
    for (i = 0; i < faces.length(); i++)
    {
        MxFace& f = slim.model().face(faces(i));

        for (j = 0; j < 3; j++)
        {
            for (k = 0; k < vertices.length(); k++)
            {
                if (vertices(k) == f(j))
                    break;
            }
            if (k < vertices.length())
                continue;

            vertices.add(f(j));
        }
    }

    MxBlock<MxVertexID> vmap(slim.model().vert_count());  // Maps old VIDs to new VIDs
    MxVertexID next_vert = 0;

    for (i = 0; i < vertices.length(); i++)
    {
        vmap(vertices(i)) = next_vert++;

        MxVertex& mv = slim.model().vertex(vertices(i));

        x = mv(X);
        y = mv(Y);
        z = mv(Z);

        if (cur_vert->elem_filled == cur_vert->nelem) {
            /* Reallocate storage if needed */
            cur_vert = get_next_block(cur_vert);
            if (cur_vert == NULL) {
                return NULL;
            }
        }
        BLOCK_LIST_TAIL(cur_vert, vertex_t).x = x;
        BLOCK_LIST_TAIL(cur_vert, vertex_t).y = y;
        BLOCK_LIST_TAIL_INCR(cur_vert, vertex_t).z = z;
        nvtcs++;

        if (x < bbmin.x) bbmin.x = x;
        if (x > bbmax.x) bbmax.x = x;
        if (y < bbmin.y) bbmin.y = y;
        if (y > bbmax.y) bbmax.y = y;
        if (z < bbmin.z) bbmin.z = z;
        if (z > bbmax.z) bbmax.z = z;
    }

    for (i = 0; i < faces.length(); i++)
    {
        MxFace& f = slim.model().face(faces(i));

        f0 = vmap(f(0));
        f1 = vmap(f(1));
        f2 = vmap(f(2));

        if (cur_face->elem_filled == cur_face->nelem) {
            /* Reallocate storage if needed */
            cur_face = get_next_block(cur_face);
            if (cur_face == NULL) {
                return NULL;
            }
        }

        BLOCK_LIST_TAIL(cur_face, face_t).f0 = f0;
        BLOCK_LIST_TAIL(cur_face, face_t).f1 = f1;
        BLOCK_LIST_TAIL_INCR(cur_face, face_t).f2 = f2;
        nfaces++;
    }

    tmesh->bBox[0] = bbmin;
    tmesh->bBox[1] = bbmax;
    tmesh->num_vert = nvtcs;
    tmesh->num_faces = nfaces;
    tmesh->vertices = (vertex_t*)malloc(nvtcs*sizeof(vertex_t));
    tmesh->faces = (face_t*)malloc(nfaces*sizeof(face_t));
    rcode = gather_block_list(head_verts, tmesh->vertices,
        nvtcs*sizeof(vertex_t));
    rcode = gather_block_list(head_faces, tmesh->faces,
        nfaces*sizeof(face_t));
    free_block_list(&head_verts);
    free_block_list(&head_faces);

    return tmesh;
}

static struct model* create_model_from_origin(MxVdpmSlim& slim, MxFaceList& faces)
{
    struct model *tmesh;
    vertex_t bbmin, bbmax;
    int nvtcs = 0;
    int nfaces = 0;
    int f0, f1, f2;
    float x, y, z;
    int rcode = 1;
    struct block_list *head_verts, *cur_vert;
    struct block_list *head_faces, *cur_face;

    head_verts = (struct block_list*)malloc(sizeof(struct block_list));
    head_faces = (struct block_list*)malloc(sizeof(struct block_list));

    cur_vert = head_verts;
    cur_face = head_faces;

    bbmin.x = bbmin.y = bbmin.z = FLT_MAX;
    bbmax.x = bbmax.y = bbmax.z = -FLT_MAX;
    tmesh = (struct model*)calloc(1, sizeof(struct model));

    rcode = init_block_list(head_verts, sizeof(vertex_t));
    if (rcode < 0)
        return NULL;

    rcode = init_block_list(head_faces, sizeof(face_t));
    if (rcode < 0)
        return NULL;

    // Collects vertex list
    MxVertexList vertices;
    int i, j, k;

    vertices.reset();
    for (i = 0; i < faces.length(); i++)
    {
        MxFace& f = slim.model().face(faces(i));

        for (j = 0; j < 3; j++)
        {
            for (k = 0; k < vertices.length(); k++)
            {
                if (vertices(k) == f(j))
                    break;
            }
            if (k < vertices.length())
                continue;

            vertices.add(f(j));
        }
    }

    MxBlock<MxVertexID> vmap(slim.model().vert_count());  // Maps old VIDs to new VIDs
    MxVertexID next_vert = 0;

    for (i = 0; i < vertices.length(); i++)
    {
        vmap(vertices(i)) = next_vert++;

        x = slim.vertex_geoms(vertices(i))[0];
        y = slim.vertex_geoms(vertices(i))[1];
        z = slim.vertex_geoms(vertices(i))[2];

        if (cur_vert->elem_filled == cur_vert->nelem) {
            /* Reallocate storage if needed */
            cur_vert = get_next_block(cur_vert);
            if (cur_vert == NULL) {
                return NULL;
            }
        }
        BLOCK_LIST_TAIL(cur_vert, vertex_t).x = x;
        BLOCK_LIST_TAIL(cur_vert, vertex_t).y = y;
        BLOCK_LIST_TAIL_INCR(cur_vert, vertex_t).z = z;
        nvtcs++;

        if (x < bbmin.x) bbmin.x = x;
        if (x > bbmax.x) bbmax.x = x;
        if (y < bbmin.y) bbmin.y = y;
        if (y > bbmax.y) bbmax.y = y;
        if (z < bbmin.z) bbmin.z = z;
        if (z > bbmax.z) bbmax.z = z;
    }

    for (i = 0; i < faces.length(); i++)
    {
        MxFace& f = slim.model().face(faces(i));

        f0 = vmap(f(0));
        f1 = vmap(f(1));
        f2 = vmap(f(2));

        if (cur_face->elem_filled == cur_face->nelem) {
            /* Reallocate storage if needed */
            cur_face = get_next_block(cur_face);
            if (cur_face == NULL) {
                return NULL;
            }
        }

        BLOCK_LIST_TAIL(cur_face, face_t).f0 = f0;
        BLOCK_LIST_TAIL(cur_face, face_t).f1 = f1;
        BLOCK_LIST_TAIL_INCR(cur_face, face_t).f2 = f2;
        nfaces++;
    }

    tmesh->bBox[0] = bbmin;
    tmesh->bBox[1] = bbmax;
    tmesh->num_vert = nvtcs;
    tmesh->num_faces = nfaces;
    tmesh->vertices = (vertex_t*)malloc(nvtcs*sizeof(vertex_t));
    tmesh->faces = (face_t*)malloc(nfaces*sizeof(face_t));
    rcode = gather_block_list(head_verts, tmesh->vertices,
        nvtcs*sizeof(vertex_t));
    rcode = gather_block_list(head_faces, tmesh->faces,
        nfaces*sizeof(face_t));
    free_block_list(&head_verts);
    free_block_list(&head_faces);

    return tmesh;
}

void MxVdpmPairContraction::update_deviation(const MxDynBlock<MxVdpmPairContraction>& history, MxVdpmSlim& slim)
{
    MxFaceList& model1_faces = slim.model().neighbors(v1);
    MxFaceList model2_faces;
    struct model_error model1, model2;
    struct dist_surf_surf_stats stats;
    double bbox2_diag;
    double abs_sampling_step, abs_sampling_dens;
    float r[3], n1[3], n2[3];

    dir_error = 0.0f;

    model2_faces.reset();

    if (child_v1 != UINT_MAX)
        collect_neighbors_from_child(history, history(child_v1), slim, model2_faces);
    else
        collect_neighbors_from_vertex(v1, slim, model2_faces);

    if (child_v2 != UINT_MAX)
        collect_neighbors_from_child(history, history(child_v2), slim, model2_faces);
    else
        collect_neighbors_from_vertex(v2, slim, model2_faces);

    memset(&model1, 0, sizeof(model1));
    memset(&model2, 0, sizeof(model2));

    model1.mesh = create_model_from_simplified(slim, model1_faces);
    model2.mesh = create_model_from_origin(slim, model2_faces);

    bbox2_diag = dist_v(&model2.mesh->bBox[0], &model2.mesh->bBox[1]);
    abs_sampling_step = 0.2*bbox2_diag;
    abs_sampling_dens = 1 / (abs_sampling_step*abs_sampling_step);

    dist_surf_surf(&model1, model2.mesh, abs_sampling_dens, -1, &stats, 0, NULL);

    __free_raw_model(model1.mesh);
    free_face_error(model1.fe);
    __free_raw_model(model2.mesh);

    n1[0] = stats.max_dist_vec.x;
    n1[1] = stats.max_dist_vec.y;
    n1[2] = stats.max_dist_vec.z;

    MxNormal& mn = slim.model().normal(v1);
    n2[0] = mn[0];
    n2[1] = mn[1];
    n2[2] = mn[2];
    mxv_cross3(r, n1, n2);

    uni_error = r[0] * r[0] + r[1] * r[1] + r[2] * r[2];
    dir_error = mxv_dot(n1, n2, 3);
    dir_error *= dir_error;
}

void MxVdpmPairContraction::collect_neighbors_from_child(const MxDynBlock<MxVdpmPairContraction>& history,
    const MxVdpmPairContraction& conx, MxVdpmSlim& slim, MxFaceList& faces)
{
    if (conx.child_v1 != UINT_MAX)
        collect_neighbors_from_child(history, history(conx.child_v1), slim, faces);
    else
        collect_neighbors_from_vertex(v1, slim, faces);

    if (conx.child_v2 != UINT_MAX)
        collect_neighbors_from_child(history, history(conx.child_v2), slim, faces);
    else
        collect_neighbors_from_vertex(v2, slim, faces);
}

void MxVdpmPairContraction::collect_neighbors_from_vertex(MxVertexID v, MxVdpmSlim& slim, MxFaceList& faces)
{
    const MxFaceList& vertex_neighbors = slim.vertex_neighbors(v);

    for (int i = 0; i < vertex_neighbors.length(); i++)
    {
        int j;
        for (j = 0; j < faces.length(); j++)
        {
            if (vertex_neighbors(i) == faces(j))
                break;
        }
        if (j < faces.length())
            continue;

        faces.add(vertex_neighbors(i));
    }
}

static struct model* create_model_from_delta_faces(MxVdpmSlim& slim, MxFaceList& faces, VID v1, VID v2)
{
    struct model *tmesh;
    vertex_t bbmin, bbmax;
    int nvtcs = 0;
    int nfaces = 0;
    int f0, f1, f2;
    float x, y, z;
    int rcode = 1;
    struct block_list *head_verts, *cur_vert;
    struct block_list *head_faces, *cur_face;

    head_verts = (struct block_list*)malloc(sizeof(struct block_list));
    head_faces = (struct block_list*)malloc(sizeof(struct block_list));

    cur_vert = head_verts;
    cur_face = head_faces;

    bbmin.x = bbmin.y = bbmin.z = FLT_MAX;
    bbmax.x = bbmax.y = bbmax.z = -FLT_MAX;
    tmesh = (struct model*)calloc(1, sizeof(struct model));

    rcode = init_block_list(head_verts, sizeof(vertex_t));
    if (rcode < 0)
        return NULL;

    rcode = init_block_list(head_faces, sizeof(face_t));
    if (rcode < 0)
        return NULL;

    // Collects vertex list
    MxVertexList vertices;
    int i, j, k;

    vertices.reset();
    for (i = 0; i < faces.length(); i++)
    {
        MxFace& f = slim.model().face(faces(i));

        for (j = 0; j < 3; j++)
        {
            if (f(j) == v1 || f(j) == v2)
            {
                vertices.add(v1);
                continue;
            }

            for (k = 0; k < vertices.length(); k++)
            {
                if (vertices(k) == f(j))
                    break;
            }
            if (k < vertices.length())
                continue;

            vertices.add(f(j));
        }
    }

    MxBlock<MxVertexID> vmap(slim.model().vert_count());  // Maps old VIDs to new VIDs
    MxVertexID next_vert = 0;

    for (i = 0; i < vertices.length(); i++)
    {
        vmap(vertices(i)) = next_vert++;

        MxVertex& mv = slim.model().vertex(vertices(i));

        x = mv(X);
        y = mv(Y);
        z = mv(Z);

        if (cur_vert->elem_filled == cur_vert->nelem) {
            /* Reallocate storage if needed */
            cur_vert = get_next_block(cur_vert);
            if (cur_vert == NULL) {
                return NULL;
            }
        }
        BLOCK_LIST_TAIL(cur_vert, vertex_t).x = x;
        BLOCK_LIST_TAIL(cur_vert, vertex_t).y = y;
        BLOCK_LIST_TAIL_INCR(cur_vert, vertex_t).z = z;
        nvtcs++;

        if (x < bbmin.x) bbmin.x = x;
        if (x > bbmax.x) bbmax.x = x;
        if (y < bbmin.y) bbmin.y = y;
        if (y > bbmax.y) bbmax.y = y;
        if (z < bbmin.z) bbmin.z = z;
        if (z > bbmax.z) bbmax.z = z;
    }

    for (i = 0; i < faces.length(); i++)
    {
        MxFace& f = slim.model().face(faces(i));

        if (f(0) == v2)
            f0 = vmap(v1);
        else
        f0 = vmap(f(0));

        if (f(1) == v2)
            f1 = vmap(v1);
        else
        f1 = vmap(f(1));

        if (f(2) == v2)
            f2 = vmap(v1);
        else
        f2 = vmap(f(2));

        if (cur_face->elem_filled == cur_face->nelem) {
            /* Reallocate storage if needed */
            cur_face = get_next_block(cur_face);
            if (cur_face == NULL) {
                return NULL;
            }
        }

        BLOCK_LIST_TAIL(cur_face, face_t).f0 = f0;
        BLOCK_LIST_TAIL(cur_face, face_t).f1 = f1;
        BLOCK_LIST_TAIL_INCR(cur_face, face_t).f2 = f2;
        nfaces++;
    }

    tmesh->bBox[0] = bbmin;
    tmesh->bBox[1] = bbmax;
    tmesh->num_vert = nvtcs;
    tmesh->num_faces = nfaces;
    tmesh->vertices = (vertex_t*)malloc(nvtcs*sizeof(vertex_t));
    tmesh->faces = (face_t*)malloc(nfaces*sizeof(face_t));
    rcode = gather_block_list(head_verts, tmesh->vertices,
        nvtcs*sizeof(vertex_t));
    rcode = gather_block_list(head_faces, tmesh->faces,
        nfaces*sizeof(face_t));
    free_block_list(&head_verts);
    free_block_list(&head_faces);

    return tmesh;
}

static bool has_vertex(MxStdModel& m, FID f, VID v)
{
    for (int i = 0; i < 3; i++)
    {
        VID vid = m.face(f)(i);
        if (vid == v)
            return true;
    }
    return false;
}

static VID find_same_vertex(MxStdModel& m, FID fa, FID fb)
{
    VID result = (VID)UINT_MAX;

    for (int i = 0; i < 3; i++)
    {
        VID va = m.face(fa)(i);

        for (int j = 0; j < 3; j++)
        {
            VID vb = m.face(fb)(j);

            if (va == vb)
            {
                result = va;
                break;
            }
        }
    }
    return result;
}

static FID find_face(MxStdModel& m, MxFaceList& faces, VID v1, VID v2, VID v3)
{
    for (int i = 0; i < faces.length(); i++)
    {
        FID fid = faces(i);

        if (!has_vertex(m, fid, v1))
            continue;

        if (!has_vertex(m, fid, v2))
            continue;

        if (!has_vertex(m, fid, v3))
            continue;

        return fid;
    }
    return (FID)UINT_MAX;
}

bool MxVdpmPairContraction::update_faces(MxVdpmSlim& slim)
{
    MxStdModel& m = slim.model();
    struct model_error model;
    struct model_info minfo;
    VID vl, vr;

    // check non-manifold ecol
    memset(&model, 0, sizeof(model));
    model.mesh = create_model_from_delta_faces(slim, delta_faces, v1, v2);
    analyze_model(model.mesh, &minfo, 0, 0, NULL, NULL);

    __free_raw_model(model.mesh);
    free_face_error(model.fe);

    if (!minfo.manifold || minfo.n_degenerate > 0 || minfo.n_disjoint_parts > 1)
        return false;

    // Get neighbors faces
    fl = fr = fn0 = fn1 = fn2 = fn3 = (FID)UINT_MAX;
    vl = vr = (VID)UINT_MAX;

#if (SAFETY >= 2)
    mxmsg_signalf(MXMSG_TRACE, "ecol[%u] {%u %u} fcount:%d", index == UINT_MAX ? 0 : index + 1, v1, v2, dead_faces.length());
#endif

    if (dead_faces.length() <= 0 || dead_faces.length() > 2)
        return false;

    for (int k = 0; k < dead_faces.length(); k++)
    {
        FID fk = dead_faces(k);
        VID vk = m.face(fk).opposite_vertex(v1, v2);
        SanityCheck(m.vertex_is_valid(vk));

        if (m.face(fk).is_inorder(vk, v1))
        {
            vl = vk;
#if (SAFETY >= 2)
            VID vk1 = m.face(fk)(0);
            VID vk2 = m.face(fk)(1);
            VID vk3 = m.face(fk)(2);

            mxmsg_signalf(MXMSG_TRACE, "fl[%u] {%u %u %u} vl:%u", fk, vk1, vk2, vk3, vl);
#endif
            fl = fk;

            // fn0, fn1
            for (int j = 0; j < delta_faces.length(); j++)
            {
                FID fj = delta_faces(j);
                assert(m.face_is_valid(fj));

#if (SAFETY >= 2)
                VID vd1 = m.face(fj)(0);
                VID vd2 = m.face(fj)(1);
                VID vd3 = m.face(fj)(2);

                mxmsg_signalf(MXMSG_TRACE, "fn_l[%u] {%u %u %u}", fj, vd1, vd2, vd3);
#endif

                if ((m.face(fj)(0) == v1 && m.face(fj)(1) == vk) ||
                    (m.face(fj)(1) == v1 && m.face(fj)(2) == vk) ||
                    (m.face(fj)(2) == v1 && m.face(fj)(0) == vk))
                {
                    if (m.face(fj).is_inorder(v1, vk))
                    {
                        SanityCheck(fn0 == (FID)UINT_MAX);
                        fn0 = fj;

#if (SAFETY >= 2)
                        mxmsg_signalf(MXMSG_TRACE, "fn0: %u", fn0);
#endif
                    }
                }
                else if ((m.face(fj)(0) == vk && m.face(fj)(1) == v2) ||
                    (m.face(fj)(1) == vk && m.face(fj)(2) == v2) ||
                    (m.face(fj)(2) == vk && m.face(fj)(0) == v2))
                {
                    if (m.face(fj).is_inorder(vk, v2))
                    {
                        SanityCheck(fn1 == (FID)UINT_MAX);
                        fn1 = fj;

#if (SAFETY >= 2)
                        mxmsg_signalf(MXMSG_TRACE, "fn1: %u", fn1);
#endif
                    }
                }
            }
        }
        else
        {
            vr = vk;
#if (SAFETY >= 2)
            VID vk1 = m.face(fk)(0);
            VID vk2 = m.face(fk)(1);
            VID vk3 = m.face(fk)(2);

            mxmsg_signalf(MXMSG_TRACE, "fr[%u] {%u %u %u} vr:%u", fk, vk1, vk2, vk3, vr);
#endif
            fr = fk;

            // fn2, fn3
            for (int j = 0; j < delta_faces.length(); j++)
            {
                FID fj = delta_faces(j);
                assert(m.face_is_valid(fj));

#if (SAFETY >= 2)
                VID vd1 = m.face(fj)(0);
                VID vd2 = m.face(fj)(1);
                VID vd3 = m.face(fj)(2);

                mxmsg_signalf(MXMSG_TRACE, "fn_r[%u] {%u %u %u}", fj, vd1, vd2, vd3);
#endif

                if ((m.face(fj)(0) == vk && m.face(fj)(1) == v1) ||
                    (m.face(fj)(1) == vk && m.face(fj)(2) == v1) ||
                    (m.face(fj)(2) == vk && m.face(fj)(0) == v1))
                {
                    if (m.face(fj).is_inorder(vk, v1))
                    {
                        SanityCheck(fn2 == (FID)UINT_MAX);
                        fn2 = fj;

#if (SAFETY >= 2)
                        mxmsg_signalf(MXMSG_TRACE, "fn2: %u", fn2);
#endif
                    }
                }
                else if ((m.face(fj)(0) == v2 && m.face(fj)(1) == vk) ||
                    (m.face(fj)(1) == v2 && m.face(fj)(2) == vk) ||
                    (m.face(fj)(2) == v2 && m.face(fj)(0) == vk))
                {
                    if (m.face(fj).is_inorder(v2, vk))
                    {
                        SanityCheck(fn3 == (FID)UINT_MAX);
                        fn3 = fj;

#if (SAFETY >= 2)
                        mxmsg_signalf(MXMSG_TRACE, "fn3: %u", fn3);
#endif
                    }
                }
            }
        }
        SanityCheck(fn0 == (FID)UINT_MAX || fn0 != fn1);
        SanityCheck(fn1 == (FID)UINT_MAX || fn1 != fn2);
        SanityCheck(fn2 == (FID)UINT_MAX || fn2 != fn3);
    }
#if 0
    if (dead_faces.length() == 1)
    {
        if (fl == (FID)UINT_MAX)
        {
            SanityCheck(vl == (VID)UINT_MAX);

            for (int j = 0; j < delta_faces.length(); j++)
            {
                FID fj = delta_faces(j);
                assert(m.face_is_valid(fj));

                if (has_vertex(m, fj, v1))
                {
                    for (int k = 0; k < delta_faces.length(); k++)
                    {
                        if (k == j)
                            continue;

                        FID fk = delta_faces(k);

                        if (has_vertex(m, fk, v2))
                        {
                            vl = find_same_vertex(m, fj, fk);
                            if (vl != (VID)UINT_MAX && vl != vr)
                            {
                                if (find_face(m, delta_faces, v1, v2, vl) != (FID)UINT_MAX)
                                    continue;

                                SanityCheck(fn0 == (FID)UINT_MAX || fn0 == fj);
                                fn0 = fj;
                                
                                SanityCheck(fn1 == (FID)UINT_MAX || fn1 == fk);
                                fn1 = fk;

                                fl = m.add_face(v1, v2, vl);
                                dead_faces.add(fl);
                                slim.valid_faces++;
#if (SAFETY >= 2)
                                mxmsg_signalf(MXMSG_TRACE, "-fn_l {%u %u %u}", v1, v2, vl);
                                mxmsg_signalf(MXMSG_TRACE, "fn0: %u", fn0);
                                mxmsg_signalf(MXMSG_TRACE, "fn1: %u", fn1);
#endif
                                return true;
                            }
                        }
                    }
                }
            }
        }
        else
        {
            SanityCheck(fr == (FID)UINT_MAX);
            SanityCheck(vr == (VID)UINT_MAX);

            for (int j = 0; j < delta_faces.length(); j++)
            {
                FID fj = delta_faces(j);
                assert(m.face_is_valid(fj));

                if (has_vertex(m, fj, v1))
                {
                    for (int k = 0; k < delta_faces.length(); k++)
                    {
                        if (k == j)
                            continue;

                        FID fk = delta_faces(k);

                        if (has_vertex(m, fk, v2))
                        {
                            VID vr = find_same_vertex(m, fj, fk);
                            if (vr != (VID)UINT_MAX && vr != vl)
                            {
                                if (find_face(m, delta_faces, v1, v2, vr) != (FID)UINT_MAX)
                                    continue;

                                SanityCheck(fn2 == (FID)UINT_MAX || fn2 == fj);
                                fn2 = fj;

                                SanityCheck(fn3 == (FID)UINT_MAX || fn3 == fk);
                                fn3 = fk;

                                fr = m.add_face(v1, vr, v2);
                                dead_faces.add(fr);
                                slim.valid_faces++;
#if (SAFETY >= 2)
                                mxmsg_signalf(MXMSG_TRACE, "-fn_r {%u %u %u}", v1, vr, v2);
                                mxmsg_signalf(MXMSG_TRACE, "fn2: %u", fn2);
                                mxmsg_signalf(MXMSG_TRACE, "fn3: %u", fn3);
#endif
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }
#endif // 0
    return true;
}
