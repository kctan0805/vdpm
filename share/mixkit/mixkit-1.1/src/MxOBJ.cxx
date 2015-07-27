/************************************************************************

  MxOBJ

  Copyright (C) 1998 Michael Garland.  See "COPYING.txt" for details.

  $Id: MxOBJ.cxx,v 1.31 2000/12/13 15:52:00 garland Exp $

 ************************************************************************/

#include "stdmix.h"
#include "mixio.h"
#include "MxOBJ.h"
#include "MxVector.h"

#define OBJ_MAXLINE 65536


MxOBJReader::cmd_entry MxOBJReader::read_cmds[] = {
    { "v", &MxOBJReader::vertex },
    { "f", &MxOBJReader::face },

    // Properties and binding -- this is the new way to handle things
    { "vn", &MxOBJReader::prop_normal },
    { "vt", &MxOBJReader::prop_texcoord },

    { NULL, NULL }
};


MxOBJReader::MxOBJReader()
    : vx_stack(Mat4::I(), 2), tx_stack(Mat4::I(), 2),
      vcorrect_stack(0, 2), vfirst_stack(1, 2)
{
    next_vertex=1;
    next_face=1;
    quad_count=0;
    poly_count=0;
    line_buffer = new char[OBJ_MAXLINE];
    unparsed_hook = NULL;
}

MxOBJReader::~MxOBJReader()
{
    delete[] line_buffer;
}

void MxOBJReader::v_xform(Vec3& v)
{
    Vec4 v2 = vx_stack.top() * Vec4(v,1);

    v[X] = v2[X]/v2[W];
    v[Y] = v2[Y]/v2[W];
    v[Z] = v2[Z]/v2[W];
}

void MxOBJReader::t_xform(Vec2& v)
{
    Vec4 v2 = tx_stack.top() * Vec4(v[X],v[Y],0,1);

    v[X] = v2[X]/v2[W];
    v[Y] = v2[Y]/v2[W];
}

unsigned int MxOBJReader::vid_xform(int id)
{
    if( id < 0 )
	id += next_vertex;
    else
	id += vcorrect_stack.top() + (vfirst_stack.top() - 1);

    return id;
}

////////////////////////////////////////////////////////////////////////

void MxOBJReader::vertex(int, char *argv[], MxStdModel& m)
{
    Vec3 v(atof(argv[0]), atof(argv[1]), atof(argv[2]));

    v_xform(v);
    next_vertex++;

    m.add_vertex((float)v[X], (float)v[Y], (float)v[Z]);
}

void MxOBJReader::prop_normal(int /*argc*/, char *argv[], MxStdModel& m)
{
    Vec3 n(atof(argv[0]), atof(argv[1]), atof(argv[2]));
    //v_xform(n);  //!!BUG: Need to transform normals appropriately
    unitize(n);

    if (!m.normal_binding())
        m.normal_binding(MX_PERVERTEX);

    m.add_normal((float)n[X], (float)n[Y], (float)n[Z]);
}

void MxOBJReader::prop_texcoord(int /*argc*/, char *argv[], MxStdModel& m)
{
    Vec2 v(atof(argv[0]), atof(argv[1]));
    t_xform(v);

    if (!m.texcoord_binding())
        m.texcoord_binding(MX_PERVERTEX);

    m.add_texcoord((float)v[X], (float)v[Y]);
}

void MxOBJReader::face(int argc, char *argv[], MxStdModel& m)
{
    if( argc == 3 )
    {
        unsigned int vertices[3], unused1, unused2;

        for (int i = 0; i < 3; ++i)
        {
            if (sscanf(argv[i], "%u", &vertices[i]) != 1)
            {
                if (sscanf(argv[i], "%u//", &vertices[i]) != 1)
                {
                    if (sscanf(argv[i], "%u//%u", &vertices[i], &unused1) != 2)
                    {
                        if (!sscanf(argv[i], "%u/%u/%u", &vertices[i], &unused1, &unused2) != 3)
                        {
                            mxmsg_signalf(MXMSG_WARN,
                                "Invalid polygon #%d.  Ignoring it.",
                                next_face);

                            poly_count++;
                            return;
                        }
                    }
                }
            }
        }
    unsigned int v0 = vid_xform(vertices[0]);
    unsigned int v1 = vid_xform(vertices[1]);
    unsigned int v2 = vid_xform(vertices[2]);

	next_face++;
	m.add_face(v0 - 1, v1 - 1, v2 - 1);
    }
    else
    {
	mxmsg_signalf(MXMSG_WARN,
		      "Input polygon #%d has more than 4 sides.  Ignoring it.",
		      next_face);

	poly_count++;
    }
}

////////////////////////////////////////////////////////////////////////

bool MxOBJReader::execute_command(const MxCmd& cmd, void *closure)
{
    MxStdModel& m = *(MxStdModel *)closure;
    int argc = cmd.phrases[0].length();
    char **argv = (char **)&cmd.phrases[0][0];

    cmd_entry *entry = &read_cmds[0];
    bool handled = false;

    while( entry->name && !handled )
        if( streq(entry->name, cmd.op) )
        {
            (this->*(entry->cmd))(argc, argv, m);
            handled = true;
        }
        else
            entry++;

    if( !handled )
        if( !unparsed_hook ||
            unparsed_hook && !(*unparsed_hook)(cmd.op, argc, argv, m) )
        {
            return false;
        }

    return true;
}

void MxOBJReader::parse_line(char *line, MxStdModel& m)
{
    MxCmdParser::parse_line(line, &m);
}

MxStdModel *MxOBJReader::read(istream& in, MxStdModel *m)
{
    if( !m )
	m = new MxStdModel(8, 16);

    while( !in.eof() )
    {
	in >> ws;
	if( in.peek() == '#' )
	    in.ignore(OBJ_MAXLINE, '\n');
	else if( in.getline(line_buffer, OBJ_MAXLINE, '\n').good() )
	    parse_line(line_buffer, *m);
    }

    if( quad_count )
	mxmsg_signalf(MXMSG_WARN,
		      "Split %d input quadrilaterals.  "
		      "Auto-splitting does not preserve properties!",
		      quad_count);
    if( poly_count )
	mxmsg_signalf(MXMSG_WARN,
		      "Ignored %d input polygons of more than 4 sides.",
		      poly_count);


    return m;
}



MxOBJWriter::MxOBJWriter()
{
    vertex_annotate = NULL;
    face_annotate = NULL;
}

void MxOBJWriter::write(ostream& out, MxStdModel& m)
{
    unsigned int i;

    for(unsigned int v=0; v<m.vert_count(); v++)
    {
	// We must output all vertices since numbering is implicit
	if( vertex_annotate )  (*vertex_annotate)(out, m, v);
	out << m.vertex(v) << endl;
    }

    for(unsigned int f=0; f<m.face_count(); f++)
	if( m.face_is_valid(f) )
	{
	    if( face_annotate ) (*face_annotate)(out, m, f);
	    out << m.face(f) << endl;
	}

    if( m.normal_binding() != MX_UNBOUND )
    {
	for(i=0; i<m.normal_count(); i++)
	    out << m.normal(i) << endl;
    }

    if( m.texcoord_binding() != MX_UNBOUND )
    {
	for(i=0; i<m.texcoord_count(); i++)
	    out << m.texcoord(i) << endl;
    }
}
