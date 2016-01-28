/************************************************************************

  Specialized QSlim output routines.  The bulk of the code here is to
  support multiresolution output formats.

  Copyright (C) 1998 Michael Garland.  See "COPYING.txt" for details.

  $Id: output.cxx,v 1.4 1999/10/18 15:55:10 garland Exp $

 ************************************************************************/

#include <cstdint>
#include <stdmix.h>
#include "qslim.h"

static
void output_ivrml(ostream& out, bool vrml=false)
{
    uint i;

    if( vrml )
	out << "#VRML V1.0 ascii" << endl;
    else
	out << "#Inventor V2.0 ascii" << endl;

    out << "Separator {" << endl
         << "Coordinate3 {" << endl
         << "point [" << endl;

    for(i=0; i<m->vert_count(); i++)
	if( m->vertex_is_valid(i) )
	    out << "   " << m->vertex(i)[0] << " "
		<< m->vertex(i)[1] << " "
		<< m->vertex(i)[2] << "," << endl;

    out << "]"<< endl << "}" << endl;
    out << "IndexedFaceSet {" << endl
         << "coordIndex [" << endl;

    for(i=0; i<m->face_count(); i++)
	if( m->face_is_valid(i) )
	    out << "   "
		<< m->face(i)[0] << ", "
		<< m->face(i)[1] << ", "
		<< m->face(i)[2] << ", "
		<< "-1," << endl;

    out << "]}}" << endl;
}

void output_iv(ostream& out) { output_ivrml(out, false); }
void output_vrml(ostream& out) { output_ivrml(out, true); }

void output_regressive_mmf(ostream& out)
{
    if( !history ) return;

    out << "set delta_encoding 1" << endl;

    for(uint i=0; i<history->length(); i++)
    {
        MxPairContraction& conx = (*history)[i];

	// Output the basic contraction record
        out << "v% " << conx.v1+1 << " " << conx.v2+1 << " "
            << conx.dv1[X] << " " << conx.dv1[Y] << " " << conx.dv1[Z]
            << endl;

        // Output the faces that are being removed
        for(uint j=0; j<conx.dead_faces.length(); j++)
            out << "f- " << conx.dead_faces(j)+1 << endl;
    }
}

void output_regressive_log(ostream& out)
{
    if( !history ) return;

    for(uint i=0; i<history->length(); i++)
    {
        MxPairContraction& conx = (*history)[i];

	// Output the basic contraction record
        out << "v% " << conx.v1+1 << " " << conx.v2+1 << " "
            << conx.dv1[X] << " " << conx.dv1[Y] << " " << conx.dv1[Z];

        // Output the faces that are being removed
        for(uint j=0; j<conx.dead_faces.length(); j++)
            out << " " << conx.dead_faces(j)+1;

        // Output the faces that are being reshaped
        out << " &";
        for(uint k=0; k<conx.delta_faces.length(); k++)
            out << " " << conx.delta_faces(k)+1;

        out << endl;
    }
}

void output_progressive_pm(ostream& out)
{
    if( !history ) return;

    MxBlock<MxVertexID> vmap(m->vert_count());  // Maps old VIDs to new VIDs
    MxBlock<MxFaceID> fmap(m->face_count());    // Maps old FIDs to new FIDs
    uint i,k;

    MxVertexID next_vert = 0;
    MxFaceID   next_face = 0;

    ////////////////////////////////////////////////////////////////////////
    //
    // Output base mesh
    //
    for(i=0; i<m->vert_count(); i++)
	if( m->vertex_is_valid(i) )
	{
	    vmap(i) = next_vert++;
	    out << m->vertex(i) << endl;
	}

    for(i=0; i<m->face_count(); i++)
	if( m->face_is_valid(i) )
	{
	    fmap(i) = next_face++;
	    VID v1 = vmap(m->face(i)(0));
	    VID v2 = vmap(m->face(i)(1));
	    VID v3 = vmap(m->face(i)(2));

	    out << "f " << v1+1 << " " << v2+1 << " " << v3+1 << endl;
	}

    ////////////////////////////////////////////////////////////////////////
    //
    // Output mesh expansion
    //
    for(i=history->length()-1; i<=history->length(); i--)
    {
	const MxPairContraction& conx = (*history)[i];
	SanityCheck( m->vertex_is_valid(conx.v1) );
	SanityCheck( !m->vertex_is_valid(conx.v2) );

	out << "v* " << vmap(conx.v1) + 1;
	out << "  "<<conx.dv1[X]<<" "<<conx.dv1[Y]<<" "<<conx.dv1[Z];
	out << "  "<<conx.dv2[X]<<" "<<conx.dv2[Y]<<" "<<conx.dv2[Z];
	out << " ";

	// Output new faces
	for(k=0; k<conx.dead_faces.length(); k++)
	{
	    FID fk = conx.dead_faces(k);
	    VID vk = m->face(fk).opposite_vertex(conx.v1, conx.v2);
 	    SanityCheck( m->vertex_is_valid(vk) );

 	    out << " ";
	    if( !m->face(fk).is_inorder(vk, conx.v1) ) out << "-";
 	    out << vmap(vk)+1;

	    fmap(conx.dead_faces(k)) = next_face++;
	    m->face_mark_valid(conx.dead_faces(k));
	}

	// Output delta faces
	out << " &";
	for(k=0; k<conx.delta_faces.length(); k++)
	{
	    out << " ";
	    FID fk = conx.delta_faces(k);
	    assert(m->face_is_valid(fk));
	    out << " " << fmap(fk)+1;
	}

	vmap(conx.v2) = next_vert++;
	m->vertex_mark_valid(conx.v2);
	out << endl;
    }
}

void output_progressive_vdpm(ostream& out)
{
    #define VDPM_FILE_FORMAT_MAGIC \
        ((long)'v' + ((long)'d' << 8) + ((long)'p' << 16) + ((long)'m' << 24))

    #define VDPM_FILE_FORMAT_SRMESH     0x00000001
    #define VDPM_FILE_FORMAT_TEXNAME    0x00000002
    #define VDPM_FILE_FORMAT_END        0x00000003
    #define VDPM_HAS_COLOR              0x00000001
    #define VDPM_HAS_TEXCOORD           0x00000002

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

    if( !history ) return;

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
    for(i=0; i<m->vert_count(); i++)
    {
	    if( m->vertex_is_valid(i) )
	    {
	        index = vmap(i) = next_vert++;

            // Get base geometries
		    VGeom& vgeom = vgeoms(index);

		    vgeom.point[0] = m->vertex(i)[0];
		    vgeom.point[1] = m->vertex(i)[1];
		    vgeom.point[2] = m->vertex(i)[2];

		    if( slim->use_color )
		    {
			    vgeom.color[0] = m->color(i).R();
			    vgeom.color[1] = m->color(i).G();
			    vgeom.color[2] = m->color(i).B();
		    }
            else
            {
                vgeom.color[0] = vgeom.color[1] = vgeom.color[2] = 0.0f;
            }

		    if( slim->use_normals )
		    {
			    vgeom.normal[0] = m->normal(i)[0];
			    vgeom.normal[1] = m->normal(i)[1];
			    vgeom.normal[2] = m->normal(i)[2];
		    }
            else
            {
                vgeom.normal[0] = vgeom.normal[1] = vgeom.normal[2] = 0.0f;
            }

		    if( slim->use_texture )
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

	for(i=0; i<(int)slim->valid_faces; i++)
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
		SanityCheck( m->vertex_is_valid(conx.v1) );
		SanityCheck( !m->vertex_is_valid(conx.v2) );
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
		if( slim->use_color )
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

        if( slim->use_normals )
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
		if( slim->use_color )
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

        if( slim->use_normals )
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
		for(k=0; k<(uint)conx.delta_faces.length(); k++)
		{
			FID fk = conx.delta_faces(k);
			assert(m->face_is_valid(fk));
			m->face(fk).remap_vertex(conx.v1, conx.v2);
		}
		vmap(conx.v2) = next_vert++;
		m->vertex_mark_valid(conx.v2);
		j++;
    }

    uint32_t valu;
    float valf;

    // Output header magic
    valu = VDPM_FILE_FORMAT_MAGIC;
    out.write((char*)&valu, sizeof(uint32_t));

    // Output VDPM data
    valu = VDPM_FILE_FORMAT_SRMESH;
    out.write((char*)&valu, sizeof(uint32_t));

    // Output flags
    valu = 0;

    if (m->color_binding())
        valu |= VDPM_HAS_COLOR;

    if (m->texcoord_binding())
        valu |= VDPM_HAS_TEXCOORD;

    out.write((char*)&valu, sizeof(uint32_t));

    // Output bounds
    for (i = 0; i < 3; i++)
    {
        valf = slim->bounds.min[i];
        out.write((char*)&valf, sizeof(float));
    }

    for (i = 0; i < 3; i++)
    {
        valf = slim->bounds.max[i];
        out.write((char*)&valf, sizeof(float));
    }

	// Output base vertex counts
    valu = slim->valid_verts;
    out.write((char*)&valu, sizeof(uint32_t));

	// Output base face counts
    valu = slim->valid_faces;
    out.write((char*)&valu, sizeof(uint32_t));

	// Output vertex split counts
    valu = history->length();
    out.write((char*)&valu, sizeof(uint32_t));

#if (SAFETY >= 2)
    mxmsg_signalf(MXMSG_DEBUG, "base_vcount: %u", slim->valid_verts);
    mxmsg_signalf(MXMSG_DEBUG, "base_fcount: %u", slim->valid_faces);
    mxmsg_signalf(MXMSG_DEBUG, "vsplit_count: %u", history->length());
#endif

	// Output vertices
	for (i=0; i<vcount;i++)
	{
		Vertex& v = vertices(i);

        valu = v.parent;
        out.write((char*)&valu, sizeof(uint32_t));

        valu = v.i;
        out.write((char*)&valu, sizeof(uint32_t));

    #if (SAFETY >= 2)
        mxmsg_signalf(MXMSG_DEBUG, "v[%u] p:%d i:%d", i, v.parent, v.i);
    #endif
	}

	// Output geometries
	for (i=0; i<vcount;i++)
	{
		VGeom& g = vgeoms(i);

        for (j = 0; j < 3; j++)
        {
            valf = g.point[j];
            out.write((char*)&valf, sizeof(float));
        }

        for (j = 0; j < 3; j++)
        {
            valf = g.normal[j];
            out.write((char*)&valf, sizeof(float));
        }

        if (slim->use_color)
        {
            for (j = 0; j < 3; j++)
            {
                valf = g.color[j];
                out.write((char*)&valf, sizeof(float));
            }
        }

        if (slim->use_texture)
        {
            valf = g.tu;
            out.write((char*)&valf, sizeof(float));

            valf = g.tv;
            out.write((char*)&valf, sizeof(float));
        }

    #if (SAFETY >= 2)
        mxmsg_signalf(MXMSG_DEBUG, "g[%u] p:{%f %f %f} n:{%f %f %f} c:{%f %f %f} t:{%f %f}", i, g.point[0], g.point[1], g.point[2],
            g.normal[0], g.normal[1], g.normal[2], g.color[0], g.color[1], g.color[2], g.tu, g.tv);
    #endif
	}

    // Output faces
    for (i = 0; i<(int)slim->valid_faces; i++)
    {
        Face& f = faces(i);

        for (j = 0; j < 3; j++)
        {
            valu = f.vertices[j];
            out.write((char*)&valu, sizeof(uint32_t));
        }

    #if (SAFETY >= 2)
        mxmsg_signalf(MXMSG_DEBUG, "f[%u] {%u %u %u}", i, f.vertices[0], f.vertices[1], f.vertices[2]);
    #endif
    }

    // Output neighbor faces
    for (i = 0; i<(int)slim->valid_faces; i++)
    {
        Face& f = faces(i);

        for (j = 0; j < 3; j++)
        {
            valu = f.neighbors[j];
            out.write((char*)&valu, sizeof(uint32_t));
        }

    #if (SAFETY >= 2)
        mxmsg_signalf(MXMSG_DEBUG, "nf[%u] {%d %d %d}", i, f.neighbors[0], f.neighbors[1], f.neighbors[2]);
    #endif
    }

	// Output vertex splits
	for (i=0; i<(uint)history->length();i++)
	{
		Vsplit& s = vsplits(i);

        for (j = 0; j < 4; j++)
        {
            valu = s.fn[j];
            out.write((char*)&valu, sizeof(uint32_t));
        }

        valf = s.radius;
        out.write((char*)&valf, sizeof(float));

        valf = s.sin2alpha;
        out.write((char*)&valf, sizeof(float));

        valf = s.uni_error;
        out.write((char*)&valf, sizeof(float));

        valf = s.dir_error;
        out.write((char*)&valf, sizeof(float));

    #if (SAFETY >= 2)
        mxmsg_signalf(MXMSG_DEBUG, "s[%u] {%u %u} fn:{%d %d %d %d} e:{%f %f %f %f}", i, s.vt_i, s.vu_i,
            s.fn[0], s.fn[1], s.fn[2], s.fn[3], s.radius, s.sin2alpha, s.uni_error, s.dir_error);
    #endif
	}

    // Output texture filename if exists
    if (m->texmap_name())
    {
        const char* texname = m->texmap_name();
        int len = strlen(texname) + 1;
        int gap = 4 - len % 4;

        if (gap == 4)
            gap = 0;

        valu = VDPM_FILE_FORMAT_TEXNAME;
        out.write((char*)&valu, sizeof(uint32_t));

        valu = len + gap;
        out.write((char*)&valu, sizeof(uint32_t));

        out.write((char*)texname, len);

        valu = 0;
        out.write((char*)&valu, gap);
    }

    // Output end token
    valu = VDPM_FILE_FORMAT_END;
    out.write((char*)&valu, sizeof(uint32_t));
}
