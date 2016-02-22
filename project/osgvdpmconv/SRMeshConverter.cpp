#include <fstream>
#include "osg/Geode"
#include "osg/Geometry"
#include "vdpm/Allocator.h"
#include "SRMeshConverter.h"

using namespace osg;

typedef MxDynBlock<MxVdpmPairContraction> QSlimLog;
static MxVdpmSlim* slim;
static QSlimLog* history;
static ostream *debug_stream;

static bool mxmsg_debug_handler(MxMsgInfo *info)
{
    *debug_stream << mxmsg_severity_name(info->severity) << ": ";
    *debug_stream << info->message << endl;
    if (info->context)
    {
        *debug_stream << "  [Location: " << info->context << "]" << endl;
    }
    if (info->filename)
    {
        *debug_stream << "  [File: " << info->filename
            << " at line " << info->line << "]" << endl;
    }
    debug_stream->flush();
    return true;
}

SRMeshConverter::SRMeshConverter(unsigned int faceTarget, std::string& fileName) : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
{
    this->faceTarget = faceTarget;

#if (SAFETY > 0)
    if (!debug_stream)
    {
        debug_stream = new ofstream(fileName + ".dbg");
    }
    mxmsg_set_handler(mxmsg_debug_handler);
    mxmsg_severity_level(MXMSG_TRACE);
#endif
}

SRMeshConverter::~SRMeshConverter()
{
    delete debug_stream;
}

class SRMeshConv : public vdpm::SRMesh
{
public:
    void write(MxVdpmSlim* slim, MxStdModel* m);
};

void SRMeshConv::write(MxVdpmSlim* slim, MxStdModel* m)
{
    struct AVertex;

    struct VGeom
    {
        float point[3];
        float normal[3];
        float color[3];
        float tu, tv;
    };

    struct Vertex
    {
        AVertex* avertex;
        uint parent;
        uint i;
    };

    struct Face
    {
        uint vertices[3];
        uint neighbors[3];
    };

    struct Vsplit
    {
        uint vt_i;
        uint vu_i;
        uint fn[4];
        float radius;
        float sin2alpha;
        float uni_error;
        float dir_error;
    };

    uint vcount = slim->valid_verts + history->length() * 2;
    MxBlock<VGeom> vgeoms(vcount);
    MxBlock<Face> faces(slim->valid_faces);
    MxBlock<Vertex> vertices(vcount);
    MxBlock<Vsplit> vsplits(history->length());

    MxBlock<MxVertexID> vmap(m->vert_count());  // Maps old VIDs to new VIDs
    MxBlock<MxFaceID> fmap(m->face_count());    // Maps old FIDs to new FIDs
    uint index, i, j, k;

    MxVertexID next_vert = 0;
    MxFaceID   next_face = 0;

    // Update refine informations
    for (i = 0; i<(uint)history->length(); i++)
    {
        MxVdpmPairContraction& conx = (*history)[i];
        conx.update_radius(*history, slim->vertex_radiuses, m);
        conx.update_cone(*history, *slim);
    }

    // Get base vertex
    for (i = 0; i<m->vert_count(); i++)
    {
        if (m->vertex_is_valid(i))
        {
            index = vmap(i) = next_vert++;

            // Get base geometries
            VGeom& vgeom = vgeoms(index);

            vgeom.point[0] = m->vertex(i)[0];
            vgeom.point[1] = m->vertex(i)[1];
            vgeom.point[2] = m->vertex(i)[2];

            if (slim->use_color)
            {
                vgeom.color[0] = m->color(i).R();
                vgeom.color[1] = m->color(i).G();
                vgeom.color[2] = m->color(i).B();
            }
            else
            {
                vgeom.color[0] = vgeom.color[1] = vgeom.color[2] = 0.0f;
            }

            if (slim->use_normals)
            {
                vgeom.normal[0] = m->normal(i)[0];
                vgeom.normal[1] = m->normal(i)[1];
                vgeom.normal[2] = m->normal(i)[2];
            }
            else
            {
                vgeom.normal[0] = vgeom.normal[1] = vgeom.normal[2] = 0.0f;
            }

            if (slim->use_texture)
            {
                vgeom.tu = m->texcoord(i)[0];
                vgeom.tv = m->texcoord(i)[1];
            }
            else
            {
                vgeom.tu = vgeom.tv = 0.0f;
            }

            // Get base vertices
            Vertex& vertex = vertices(index);

            vertex.avertex = NULL;
            vertex.parent = UINT_MAX;
            vertex.i = UINT_MAX;

            for (j = history->length() - 1; j <= (uint)history->length(); j--)
            {
                MxVdpmPairContraction& conx = (*history)[j];
                if (m->vertex_is_valid(conx.v1) && i == conx.v1)
                {
                    conx.v_i = index;
                    break;
                }
            }
        }
    }

    // Get base faces
    for (i = 0; i < m->face_count(); i++)
    {
        if (m->face_is_valid(i))
        {
            index = fmap(i) = next_face++;
            Face& face = faces(index);

            face.vertices[0] = vmap(m->face(i)(0));
            face.vertices[1] = vmap(m->face(i)(1));
            face.vertices[2] = vmap(m->face(i)(2));
        }
    }

    // Get neighbors of base faces
    MxBlock<VID[3]> face_neighbors(slim->valid_faces);

    for (i = 0; i<(int)slim->valid_faces; i++)
        faces(i).neighbors[0] = faces(i).neighbors[1] = faces(i).neighbors[2] = (VID)UINT_MAX;

    for (i = 0; i < m->face_count(); i++)
    {
        if (m->face_is_valid(i))
        {
            for (k = i + 1; k < (int)m->face_count(); k++)
            {
                if (m->face_is_valid(k))
                {
                    if (m->face(i)(0) == m->face(k)(1) &&
                        m->face(i)(1) == m->face(k)(0))
                    {
                        faces(fmap(i)).neighbors[0] = fmap(k);
                        faces(fmap(k)).neighbors[0] = fmap(i);
                    }
                    else if (m->face(i)(0) == m->face(k)(0) &&
                        m->face(i)(1) == m->face(k)(2))
                    {
                        faces(fmap(i)).neighbors[0] = fmap(k);
                        faces(fmap(k)).neighbors[2] = fmap(i);
                    }
                    else if (m->face(i)(0) == m->face(k)(2) &&
                        m->face(i)(1) == m->face(k)(1))
                    {
                        faces(fmap(i)).neighbors[0] = fmap(k);
                        faces(fmap(k)).neighbors[1] = fmap(i);
                    }

                    if (m->face(i)(2) == m->face(k)(1) &&
                        m->face(i)(0) == m->face(k)(0))
                    {
                        faces(fmap(i)).neighbors[2] = fmap(k);
                        faces(fmap(k)).neighbors[0] = fmap(i);
                    }
                    else if (m->face(i)(2) == m->face(k)(0) &&
                        m->face(i)(0) == m->face(k)(2))
                    {
                        faces(fmap(i)).neighbors[2] = fmap(k);
                        faces(fmap(k)).neighbors[2] = fmap(i);
                    }
                    else if (m->face(i)(2) == m->face(k)(2) &&
                        m->face(i)(0) == m->face(k)(1))
                    {
                        faces(fmap(i)).neighbors[2] = fmap(k);
                        faces(fmap(k)).neighbors[1] = fmap(i);
                    }

                    if (m->face(i)(1) == m->face(k)(1) &&
                        m->face(i)(2) == m->face(k)(0))
                    {
                        faces(fmap(i)).neighbors[1] = fmap(k);
                        faces(fmap(k)).neighbors[0] = fmap(i);
                    }
                    else if (m->face(i)(1) == m->face(k)(0) &&
                        m->face(i)(2) == m->face(k)(2))
                    {
                        faces(fmap(i)).neighbors[1] = fmap(k);
                        faces(fmap(k)).neighbors[2] = fmap(i);
                    }
                    else if (m->face(i)(1) == m->face(k)(2) &&
                        m->face(i)(2) == m->face(k)(1))
                    {
                        faces(fmap(i)).neighbors[1] = fmap(k);
                        faces(fmap(k)).neighbors[1] = fmap(i);
                    }
                }
            }
        }
    }

    // Get vertices
    for (i = slim->valid_verts; i<vcount; i++)
    {
        Vertex& vertex = vertices(i);

        vertex.avertex = NULL;
        vertex.parent = UINT_MAX;
        vertex.i = UINT_MAX;
    }

    // Get vertex splits
    j = 0;
    for (i = history->length() - 1; i <= (uint)history->length(); i--)
    {
        const MxVdpmPairContraction& conx = (*history)[i];
        SanityCheck(m->vertex_is_valid(conx.v1));
        SanityCheck(!m->vertex_is_valid(conx.v2));
        Vsplit& vsplit = vsplits(j);
        Vertex* vertex;

        // Get vt & vu
        index = slim->valid_verts + j * 2;

        vsplit.vt_i = index;
        vsplit.vu_i = index + 1;

        // Get vertex geometries
        VGeom* vgeom = &vgeoms(index);

        vgeom->point[0] = (float)conx.vt[0];
        vgeom->point[1] = (float)conx.vt[1];
        vgeom->point[2] = (float)conx.vt[2];

        k = 3;
        if (slim->use_color)
        {
            vgeom->color[0] = (float)conx.vt[k];
            vgeom->color[1] = (float)conx.vt[k + 1];
            vgeom->color[2] = (float)conx.vt[k + 2];
            k += 3;
        }
        else
        {
            vgeom->color[0] = vgeom->color[1] = vgeom->color[2] = 0.0f;
        }

        if (slim->use_texture)
        {
            vgeom->tu = conx.vt[k++];
            vgeom->tv = conx.vt[k++];
        }
        else
        {
            vgeom->tu = vgeom->tv = 0.0f;
        }

        if (slim->use_normals)
        {
            float n[3];  n[0] = (float)conx.vt[k++];  n[1] = (float)conx.vt[k++];  n[2] = (float)conx.vt[k++];
            mxv_unitize(n, 3);
            vgeom->normal[0] = n[0];
            vgeom->normal[1] = n[1];
            vgeom->normal[2] = n[2];
        }
        else
        {
            vgeom->normal[0] = vgeom->normal[1] = vgeom->normal[2] = 0.0f;
        }

        vgeom = &vgeoms(index + 1);

        vgeom->point[0] = (float)conx.vu[0];
        vgeom->point[1] = (float)conx.vu[1];
        vgeom->point[2] = (float)conx.vu[2];

        k = 3;
        if (slim->use_color)
        {
            vgeom->color[0] = (float)conx.vu[k];
            vgeom->color[1] = (float)conx.vu[k + 1];
            vgeom->color[2] = (float)conx.vu[k + 2];
            k += 3;
        }
        else
        {
            vgeom->color[0] = vgeom->color[1] = vgeom->color[2] = 0.0f;
        }

        if (slim->use_texture)
        {
            vgeom->tu = conx.vu[k++];
            vgeom->tv = conx.vu[k++];
        }
        else
        {
            vgeom->tu = vgeom->tv = 0.0f;
        }

        if (slim->use_normals)
        {
            float n[3];  n[0] = (float)conx.vu[k++];  n[1] = (float)conx.vu[k++];  n[2] = (float)conx.vu[k++];
            mxv_unitize(n, 3);
            vgeom->normal[0] = n[0];
            vgeom->normal[1] = n[1];
            vgeom->normal[2] = n[2];
        }
        else
        {
            vgeom->normal[0] = vgeom->normal[1] = vgeom->normal[2] = 0.0f;
        }

        // Get vertices
        SanityCheck(conx.v_i != UINT_MAX);

        vertex = &vertices(conx.v_i);
        vertex->i = j;

        vertex = &vertices(vsplit.vt_i);
        vertex->parent = conx.v_i;

        vertex = &vertices(vsplit.vu_i);
        vertex->parent = conx.v_i;

        if (conx.child_v1 != UINT_MAX)
        {
            MxVdpmPairContraction& conx_v1 = (*history)[conx.child_v1];
            conx_v1.v_i = vsplit.vt_i;
        }

        if (conx.child_v2 != UINT_MAX)
        {
            MxVdpmPairContraction& conx_v2 = (*history)[conx.child_v2];
            conx_v2.v_i = vsplit.vu_i;
        }

        // Mark valid new faces
        if (conx.fl != UINT_MAX)
        {
            fmap(conx.fl) = next_face;
            m->face_mark_valid(conx.fl);
        }
        next_face++;
        if (conx.fr != UINT_MAX)
        {
            fmap(conx.fr) = next_face;
            m->face_mark_valid(conx.fr);
        }
        next_face++;

        // Get neighbors faces
        vsplit.fn[0] = conx.fn0 == UINT_MAX ? -1 : fmap(conx.fn0);
        vsplit.fn[1] = conx.fn1 == UINT_MAX ? -1 : fmap(conx.fn1);
        vsplit.fn[2] = conx.fn2 == UINT_MAX ? -1 : fmap(conx.fn2);
        vsplit.fn[3] = conx.fn3 == UINT_MAX ? -1 : fmap(conx.fn3);

        // Get radius, sin2alpha, uni_error, dir_error
        vsplit.radius = conx.radius;
        vsplit.sin2alpha = conx.sin2alpha;
        vsplit.uni_error = conx.uni_error;
        vsplit.dir_error = conx.dir_error;

        // Remap delta faces
        for (k = 0; k<(uint)conx.delta_faces.length(); k++)
        {
            FID fk = conx.delta_faces(k);
            assert(m->face_is_valid(fk));
            m->face(fk).remap_vertex(conx.v1, conx.v2);
        }
        vmap(conx.v2) = next_vert++;
        m->vertex_mark_valid(conx.v2);
        j++;
    }
    
    vdpm::AVertex* avertex;
    vdpm::AFace* aface;
    uint32_t vt_i, vu_i;
    bool hasColor, hasTexCoord;
    unsigned int vgeomCount;

    // Output bounds
    for (i = 0; i < 3; i++)
    {
        this->boundMin[i] = slim->bounds.min[i];
    }

    for (i = 0; i < 3; i++)
    {
        this->boundMax[i] = slim->bounds.max[i];
    }

    // Output base vertex counts
    this->baseVCount = slim->valid_verts;

    // Output base face counts
    this->baseFCount = slim->valid_faces;

    // Output vertex split counts
    this->vsplitCount = history->length();

#if (SAFETY >= 2)
    mxmsg_signalf(MXMSG_DEBUG, "base_vcount: %u", slim->valid_verts);
    mxmsg_signalf(MXMSG_DEBUG, "base_fcount: %u", slim->valid_faces);
    mxmsg_signalf(MXMSG_DEBUG, "vsplit_count: %u", history->length());
#endif

    this->vcount = this->baseVCount + this->vsplitCount * 2;
    this->vertices = new vdpm::Vertex[this->vcount];
    if (!this->vertices)
        goto error;

    // Output vertices
    for (i = 0; i < this->vcount; ++i)
    {
        Vertex& v = vertices(i);

        this->vertices[i].avertex = NULL;
        index = v.parent;
        this->vertices[i].parent = (index == UINT_MAX) ? NULL : &this->vertices[index];
        this->vertices[i].i = v.i;

    #if (SAFETY >= 2)
        mxmsg_signalf(MXMSG_DEBUG, "v[%u] p:%d i:%d", i, v.parent, v.i);
    #endif
    }

    // Output geometries
    vgeomCount = this->vcount;
    hasColor = m->color_binding() ? true : false;
    hasTexCoord = m->texcoord_binding() ? true : false;
    this->geometry.create(vgeomCount, hasColor, hasTexCoord);

    for (i = 0; i < this->baseVCount; ++i)
    {
        VGeom& g = vgeoms(i);
        vdpm::VGeom* vgeom = this->geometry.getVGeom(i);

        avertex = vdpm::Allocator::getInstance().allocAVertex();

        for (j = 0; j < 3; j++)
            vgeom->point[j] = g.point[j];

        for (j = 0; j < 3; j++)
            vgeom->normal[j] = g.normal[j];

        if (hasColor)
        {
            float* ptr = this->geometry.getVGeomColor(vgeom);
            for (int j = 0; j < 3; ++j)
                *ptr++ = g.color[j];
        }
        if (hasTexCoord)
        {
            float* ptr = this->geometry.getVGeomTexCoord(vgeom);
            *ptr++ = g.tu;
            *ptr++ = g.tv;
        }

        avertex->i = i;
        this->vertices[i].avertex = avertex;
        avertex->vertex = &this->vertices[i];
        avertex->vmorph = NULL;
        this->addAVertex(avertex);

    #if (SAFETY >= 2)
        mxmsg_signalf(MXMSG_DEBUG, "g[%u] p:{%f %f %f} n:{%f %f %f} c:{%f %f %f} t:{%f %f}", i, g.point[0], g.point[1], g.point[2],
            g.normal[0], g.normal[1], g.normal[2], g.color[0], g.color[1], g.color[2], g.tu, g.tv);
    #endif
    }
    for (i = 0; i < this->vsplitCount; ++i)
    {
        VGeom* g;
        vdpm::VGeom* vgeom;

        vt_i = this->baseVCount + i * 2;
        vu_i = this->baseVCount + i * 2 + 1;

        g = &vgeoms(vt_i);
        vgeom = this->geometry.getVGeom(vt_i);

        for (j = 0; j < 3; j++)
            vgeom->point[j] = g->point[j];

        for (j = 0; j < 3; j++)
            vgeom->normal[j] = g->normal[j];

        if (hasColor)
        {
            float* ptr = this->geometry.getVGeomColor(vgeom);
            for (int j = 0; j < 3; ++j)
                *ptr++ = g->color[j];
        }
        if (hasTexCoord)
        {
            float* ptr = this->geometry.getVGeomTexCoord(vgeom);
            *ptr++ = g->tu;
            *ptr++ = g->tv;
        }

    #if (SAFETY >= 2)
        mxmsg_signalf(MXMSG_DEBUG, "g[%u] p:{%f %f %f} n:{%f %f %f} c:{%f %f %f} t:{%f %f}", vt_i, g->point[0], g->point[1], g->point[2],
            g->normal[0], g->normal[1], g->normal[2], g->color[0], g->color[1], g->color[2], g->tu, g->tv);
    #endif

        g = &vgeoms(vu_i);
        vgeom = this->geometry.getVGeom(vu_i);

        for (j = 0; j < 3; j++)
            vgeom->point[j] = g->point[j];

        for (j = 0; j < 3; j++)
            vgeom->normal[j] = g->normal[j];

        if (hasColor)
        {
            float* ptr = this->geometry.getVGeomColor(vgeom);
            for (int j = 0; j < 3; ++j)
                *ptr++ = g->color[j];
        }
        if (hasTexCoord)
        {
            float* ptr = this->geometry.getVGeomTexCoord(vgeom);
            *ptr++ = g->tu;
            *ptr++ = g->tv;
        }

    #if (SAFETY >= 2)
        mxmsg_signalf(MXMSG_DEBUG, "g[%u] p:{%f %f %f} n:{%f %f %f} c:{%f %f %f} t:{%f %f}", vu_i, g->point[0], g->point[1], g->point[2],
            g->normal[0], g->normal[1], g->normal[2], g->color[0], g->color[1], g->color[2], g->tu, g->tv);
    #endif
    }

    // Output faces
    this->fcount = this->baseFCount + this->vsplitCount * 2;
    this->faces = new vdpm::Face[this->fcount];
    if (!this->faces)
        goto error;

    for (i = 0; i < this->fcount; ++i)
        this->faces[i].aface = NULL;

    for (i = 0; i < this->baseFCount; ++i)
    {
        Face& f = faces(i);
        aface = vdpm::Allocator::getInstance().allocAFace();

        index = f.vertices[0];
        aface->v0 = this->vertices[index].avertex;
        index = f.vertices[1];
        aface->v1 = this->vertices[index].avertex;
        index = f.vertices[2];
        aface->v2 = this->vertices[index].avertex;

        this->faces[i].aface = aface;
        aface->tstrip = NULL;
        this->addAFace(aface);

    #if (SAFETY >= 2)
        mxmsg_signalf(MXMSG_DEBUG, "f[%u] {%u %u %u}", i, f.vertices[0], f.vertices[1], f.vertices[2]);
    #endif
    }

    // Output neighbor faces
    for (i = 0; i < this->baseFCount; ++i)
    {
        Face& f = faces(i);

        index = f.neighbors[0];
        this->faces[i].aface->n0 = (index == UINT_MAX) ? NULL : this->faces[index].aface;
        index = f.neighbors[1];
        this->faces[i].aface->n1 = (index == UINT_MAX) ? NULL : this->faces[index].aface;
        index = f.neighbors[2];
        this->faces[i].aface->n2 = (index == UINT_MAX) ? NULL : this->faces[index].aface;

    #if (SAFETY >= 2)
        mxmsg_signalf(MXMSG_DEBUG, "nf[%u] {%d %d %d}", i, f.neighbors[0], f.neighbors[1], f.neighbors[2]);
    #endif
    }

    // Output vertex splits
    this->vsplits = new vdpm::VSplit[this->vsplitCount];
    if (!this->faces)
        goto error;

    for (i = 0; i < this->vsplitCount; ++i)
    {
        Vsplit& s = vsplits(i);

        vt_i = this->baseVCount + i * 2;
        vu_i = this->baseVCount + i * 2 + 1;

        this->vsplits[i].vt_i = vt_i;
        this->vsplits[i].vu_i = vu_i;

        index = s.fn[0];
        this->vsplits[i].fn0 = (index == UINT_MAX) ? NULL : &this->faces[index];
        index = s.fn[1];
        this->vsplits[i].fn1 = (index == UINT_MAX) ? NULL : &this->faces[index];
        index = s.fn[2];
        this->vsplits[i].fn2 = (index == UINT_MAX) ? NULL : &this->faces[index];
        index = s.fn[3];
        this->vsplits[i].fn3 = (index == UINT_MAX) ? NULL : &this->faces[index];

        this->vsplits[i].radius = s.radius;
        this->vsplits[i].sin2alpha = s.sin2alpha;
        this->vsplits[i].uni_error = s.uni_error;
        this->vsplits[i].dir_error = s.dir_error;

    #if (SAFETY >= 2)
        mxmsg_signalf(MXMSG_DEBUG, "s[%u] {%u %u} fn:{%d %d %d %d} e:{%f %f %f %f}", i, s.vt_i, s.vu_i,
            s.fn[0], s.fn[1], s.fn[2], s.fn[3], s.radius, s.sin2alpha, s.uni_error, s.dir_error);
    #endif
    }

    return;

error:
    delete[] vsplits;
    delete[] faces;
    geometry.destroy();
    delete[] vertices;
}

static void slim_history_callback(const MxPairContraction& conx, float cost)
{
    uint i;
    MxVdpmPairContraction* pconx = (MxVdpmPairContraction*)&conx;

    pconx->index = history->length();
    pconx->child_v1 = UINT_MAX;
    pconx->child_v2 = UINT_MAX;

    for (i = history->length() - 1; i <= history->length(); i--)
    {
        MxVdpmPairContraction& child_conx = (*history)[i];

        if (pconx->child_v1 == UINT_MAX && child_conx.v1 == conx.v1)
        {
            pconx->child_v1 = child_conx.index;
            child_conx.parent = pconx->index;
        }
        else if (pconx->child_v2 == UINT_MAX && child_conx.v1 == conx.v2)
        {
            pconx->child_v2 = child_conx.index;
            child_conx.parent = pconx->index;
        }
        if (pconx->child_v1 != UINT_MAX && pconx->child_v2 != UINT_MAX)
            break;
    }
    pconx->update_deviation(*history, *slim);

    history->add((MxVdpmPairContraction&)conx);
}

void SRMeshConverter::apply(osg::Geode & geode)
{
    for (unsigned int ii = 0; ii < geode.getNumDrawables(); ++ii)
    {
        osg::ref_ptr< osg::Geometry > geometry = dynamic_cast<osg::Geometry *>(geode.getDrawable(ii));
        if (geometry.valid())
        {
            MxStdModel* m = new MxStdModel(100, 100);

            read(*m, *geometry);

            slim = new MxVdpmSlim(m);
            slim->initialize();

            history = new QSlimLog(100);
            slim->contraction_callback = slim_history_callback;

            slim->decimate(faceTarget);

            osgVdpm::SRMeshDrawable* srmeshdrawable = new osgVdpm::SRMeshDrawable;

            write(*srmeshdrawable, *slim);

            srmeshdrawable->setName(geometry->getName());
            srmeshdrawable->setStateSet(geometry->getStateSet());
            srmeshdrawable->setUserDataContainer(geometry->getUserDataContainer());
            
            geode.replaceDrawable(geometry, srmeshdrawable);

            delete history;
            delete slim;
            delete m;
        }
    }
}

void SRMeshConverter::read(MxStdModel& m, osg::Geometry& geometry)
{
    osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geometry.getVertexArray());
    if (vertices)
    {
        for (unsigned int i = 0; i < vertices->size(); ++i)
        {
            m.add_vertex((*vertices)[i].x(), (*vertices)[i].y(), (*vertices)[i].z());
        }
    }

    osg::Vec3Array* normals = dynamic_cast<osg::Vec3Array*>(geometry.getNormalArray());
    if (normals)
    {
        m.normal_binding(MX_PERVERTEX);

        for (unsigned int i = 0; i < normals->size(); ++i)
        {
            m.add_normal((*normals)[i].x(), (*normals)[i].y(), (*normals)[i].z());
        }
    }

    osg::Vec4Array* colors = dynamic_cast<osg::Vec4Array*>(geometry.getColorArray());
    if (colors)
    {
        m.color_binding(MX_PERVERTEX);

        for (unsigned int i = 0; i < colors->size(); ++i)
        {
            m.add_color((*colors)[i].r(), (*colors)[i].g(), (*colors)[i].b());
        }
    }

    osg::Vec2Array* texCoords = dynamic_cast<osg::Vec2Array*>(geometry.getTexCoordArray(0));
    if (texCoords)
    {
        m.texcoord_binding(MX_PERVERTEX);

        for (unsigned int i = 0; i < texCoords->size(); ++i)
        {
            m.add_texcoord((*texCoords)[i].x(), (*texCoords)[i].y());
        }
    }

    Geometry::PrimitiveSetList& primitives = geometry.getPrimitiveSetList();
    Geometry::PrimitiveSetList::iterator itr;
    for (itr = primitives.begin(); itr != primitives.end(); ++itr)
    {
        if ((*itr)->getMode() == PrimitiveSet::TRIANGLES)
        {
            PrimitiveSet::Type type = (*itr)->getType();
            if (type == PrimitiveSet::DrawElementsUBytePrimitiveType ||
                type == PrimitiveSet::DrawElementsUShortPrimitiveType ||
                type == PrimitiveSet::DrawElementsUIntPrimitiveType)
            {
                osg::PrimitiveSet* prim = (*itr)->asPrimitiveSet();
                unsigned int j = prim->getNumPrimitives();
                for (unsigned int i = 0; i < j; ++i)
                {
                    m.add_face(prim->index(i*3), prim->index(i*3+1), prim->index(i*3+2));
                }
            }
            else
            {
                assert(0);
            }
        }
        else if ((*itr)->getMode() == PrimitiveSet::QUADS)
        {
            PrimitiveSet::Type type = (*itr)->getType();
            if (type == PrimitiveSet::DrawElementsUBytePrimitiveType ||
                type == PrimitiveSet::DrawElementsUShortPrimitiveType ||
                type == PrimitiveSet::DrawElementsUIntPrimitiveType)
            {
                osg::PrimitiveSet* prim = (*itr)->asPrimitiveSet();
                unsigned int j = prim->getNumPrimitives();
                for (unsigned int i = 0; i < j; ++i)
                {
                    m.add_face(prim->index(i * 4), prim->index(i * 4 + 1), prim->index(i * 4 + 2));
                    m.add_face(prim->index(i * 4 + 2), prim->index(i * 4 + 3), prim->index(i * 4));
                }
            }
            else
            {
                assert(0);
            }
        }
        else
        {
            assert(0);
        }
    }
}

void SRMeshConverter::write(osgVdpm::SRMeshDrawable& srmeshdrawable, MxVdpmSlim& slim)
{
    SRMeshConv* srmeshconv = new SRMeshConv();
    if (!srmeshconv)
        return;

    MxStdModel& m = slim.model();

    srmeshconv->write(&slim, &m);
    srmeshdrawable.setSRMesh(srmeshconv);
}