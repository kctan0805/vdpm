#ifndef MXOBJ_INCLUDED // -*- C++ -*-
#define MXOBJ_INCLUDED
#if !defined(__GNUC__)
#  pragma once
#endif

/************************************************************************

  MxOBJ

  Copyright (C) 1998 Michael Garland.  See "COPYING.txt" for details.

  $Id: MxOBJ.h,v 1.20 2000/11/20 20:36:38 garland Exp $

 ************************************************************************/


// Interim reader based on OBJ 1.0
// This reader is designed to create MxStdModel models

#include "MxCmdParser.h"
#include "MxStack.h"
#include "MxStdModel.h"
#include "MxMat4.h"

class MxOBJReader : private MxCmdParser
{
public:
    typedef void (MxOBJReader::*read_cmd)(int argc,
					  char *argv[],
					  MxStdModel& m);
    struct cmd_entry { char *name; read_cmd cmd; };

private:
    MxStack<unsigned int> vfirst_stack;
    MxStack<int> vcorrect_stack;
    MxStack<Mat4> vx_stack;
    MxStack<Mat4> tx_stack;
    unsigned int next_vertex;
    unsigned int next_face;

    unsigned int quad_count;	// # of quadrilaterals in input
    unsigned int poly_count;	// # of 5+ sided polygons in input

    char *line_buffer;

    static cmd_entry read_cmds[];

private:
    bool execute_command(const MxCmd&, void *closure);

protected:
    void v_xform(Vec3& v);
    void t_xform(Vec2& v);
    unsigned int vid_xform(int id);

    void vertex(int argc, char *argv[], MxStdModel&);
    void face(int argc, char *argv[], MxStdModel&);

    void prop_normal(int argc, char *argv[], MxStdModel&);
    void prop_texcoord(int argc, char *argv[], MxStdModel&);

public:
    MxOBJReader();
    ~MxOBJReader();

    bool (*unparsed_hook)(char *cmd, int argc, char *argv[], MxStdModel& m);

    MxAspStore *asp_store() { return this->MxCmdParser::asp_store(); }

    MxStdModel *read(istream& in, MxStdModel *model=NULL);
    void parse_line(char *, MxStdModel&);
};

class MxOBJWriter
{
public:
    MxOBJWriter();

    void (*vertex_annotate)(ostream&, const MxStdModel&, MxVertexID);
    void (*face_annotate)(ostream&, const MxStdModel&, MxVertexID);

    void write(ostream& out, MxStdModel& m);
};

// MXOBJ_INCLUDED
#endif
