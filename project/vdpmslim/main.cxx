/************************************************************************

  Main QSlim file.

  Copyright (C) 1998 Michael Garland.  See "COPYING.txt" for details.
  
  $Id: main.cxx,v 1.21.2.1 2002/01/31 19:53:14 garland Exp $

 ************************************************************************/

#include <stdmix.h>
#include <mixio.h>
#include "qslim.h"

// Configuration variables and initial values
//
unsigned int face_target = 0;
int placement_policy = MX_PLACE_OPTIMAL;
double boundary_weight = 1000.0;
int weighting_policy = MX_WEIGHT_AREA;
bool will_record_history = true;
double compactness_ratio = 0.0;
double meshing_penalty = 1.0;
bool will_join_only = false;
bool be_quiet = false;
OutputFormat output_format = VDPM;
char *output_filename = NULL;
ostream *debug_stream = NULL;

// Globally visible variables
//
MxSMFReader *smf = NULL;
MxOBJReader *obj = NULL;
MxStdModel *m = NULL;
MxStdModel *m_orig = NULL;
MxVdpmSlim *slim = NULL;
QSlimLog *history = NULL;
MxDynBlock<MxEdge> *target_edges = NULL;

const char *slim_copyright_notice =
"Copyright (C) 1998-2002 Michael Garland. Copyright (C) 2015 Jim Tan.";

const char *slim_version_string = "1.0";

static ostream *output_stream = NULL;

static
bool qslim_smf_hook(char *op, int, char *argv[], MxStdModel& m)
{
    if( streq(op, "e") )
    {
	if( !target_edges )
	    target_edges = new MxDynBlock<MxEdge>(m.vert_count() * 3);

	MxEdge& e = target_edges->add();

	e.v1 = atoi(argv[0]) - 1;
	e.v2 = atoi(argv[1]) - 1;

	return true;
    }

    return false;
}

bool (*unparsed_hook)(char *, int, char*[], MxStdModel&) = qslim_smf_hook;

void slim_print_banner(ostream& out)
{
    out << "VDPMSlim surface simplification software." << endl
	<< "Version " << slim_version_string << " "
	<< "[Built " << __DATE__ << "]." << endl
	<< slim_copyright_notice << endl;
}

void slim_init()
{
    if( !slim )
    {
	    slim = new MxVdpmSlim(m);
    }

    slim->placement_policy = placement_policy;
    slim->boundary_weight = boundary_weight;
    slim->weighting_policy = weighting_policy;
    slim->compactness_ratio = compactness_ratio;
    slim->meshing_penalty = meshing_penalty;
    slim->will_join_only = will_join_only;

	slim->initialize();

    if( will_record_history )
    {
	    history = new QSlimLog(100);
	    slim->contraction_callback = slim_history_callback;
    }
}

#define CLEANUP(x)  if(x) { delete x; x=NULL; }

void slim_cleanup()
{
    CLEANUP(obj);
    CLEANUP(smf);
    CLEANUP(m);
    CLEANUP(slim);
    CLEANUP(history);
    CLEANUP(target_edges);
    if( output_stream != &cout )
    	CLEANUP(output_stream);

    if( debug_stream != &cout )
    	CLEANUP(debug_stream);
}

static
void setup_output()
{
    if( !output_stream )
    {
	if( output_filename )
        output_stream = new ofstream(output_filename, ios::out | (output_format == VDPM) ? ios::binary : 0);
	else
	    output_stream = &cout;
    }
}

bool select_output_format(const char *fmt)
{
    bool h = false;

    if     ( streq(fmt, "mmf") ) { output_format = MMF; h = true; }
    else if( streq(fmt, "pm")  ) { output_format = PM;  h = true; }
    else if( streq(fmt, "log") ) { output_format = LOG; h = true; }
    else if( streq(fmt, "smf") ) output_format = SMF;
    else if( streq(fmt, "iv")  ) output_format = IV;
    else if( streq(fmt, "vrml")) output_format = VRML;
	else if( streq(fmt, "vdpm")) { output_format = VDPM;  h = true; }
    else return false;

    if( h ) will_record_history = true;

    return true;
}

void output_preamble()
{
    if( output_format==MMF || output_format==LOG )
	output_current_model();
}

void output_current_model()
{
    setup_output();

    MxSMFWriter writer;
    writer.write(*output_stream, *m);
}

static
void cleanup_for_output()
{
    // First, mark stray vertices for removal
    //
    for(uint i=0; i<m->vert_count(); i++)
	if( m->vertex_is_valid(i) && m->neighbors(i).length() == 0 )
	    m->vertex_mark_invalid(i);

	// Compact vertex array so only valid vertices remain
    m->compact_vertices();
}

void output_final_model()
{
    setup_output();

    switch( output_format )
    {
    case MMF:
	output_regressive_mmf(*output_stream);
	break;

    case LOG:
	output_regressive_log(*output_stream);
	break;

    case PM:
	output_progressive_pm(*output_stream);
	break;

    case IV:
	cleanup_for_output();
	output_iv(*output_stream);
	break;

    case VRML:
	cleanup_for_output();
	output_vrml(*output_stream);
	break;

    case SMF:
	cleanup_for_output();
	output_current_model();
	break;

    case VDPM:
	output_progressive_vdpm(*output_stream);
	break;
    }


}

void input_file(const char *filename)
{
    char * pch = strrchr((char*)filename, '.');
    if (pch && stricmp(pch, ".obj") == 0)
    {
        obj = new MxOBJReader;
        obj->unparsed_hook = unparsed_hook;

        if (streq(filename, "-"))
            obj->read(cin, m);
        else
        {
            ifstream in(filename);
            if (!in.good())
                mxmsg_signal(MXMSG_FATAL, "Failed to open input file", filename);
            obj->read(in, m);
            in.close();
        }
    }
    else
    {
        smf = new MxSMFReader;
        smf->unparsed_hook = unparsed_hook;

        if (streq(filename, "-"))
            smf->read(cin, m);
        else
        {
            ifstream in(filename);
            if (!in.good())
                mxmsg_signal(MXMSG_FATAL, "Failed to open input file", filename);
            smf->read(in, m);
            in.close();
        }
    }
}

static
MxDynBlock<char*> files_to_include(2);

void defer_file_inclusion(char *filename)
{
    files_to_include.add(filename);
}

void include_deferred_files()
{
    for(uint i=0; i<files_to_include.length(); i++)
	input_file(files_to_include[i]);
}

void slim_history_callback(const MxPairContraction& conx, float cost)
{
	uint i;
	MxVdpmPairContraction* pconx = (MxVdpmPairContraction*)&conx;

	pconx->index = history->length();
    pconx->child_v1 = UINT_MAX;
    pconx->child_v2 = UINT_MAX;

	for(i=history->length()-1; i<=history->length(); i--)
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

#if (SAFETY >= 2)
    mxmsg_signalf(MXMSG_DEBUG, "c[%u] v:{%u %u} child:{%d %d}", pconx->index, conx.v1, conx.v2, pconx->child_v1, pconx->child_v2);
#endif

    history->add((MxVdpmPairContraction&)conx);
}
