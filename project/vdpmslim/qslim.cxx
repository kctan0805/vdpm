/************************************************************************

  QSlim command line program.  This provides a very simple interface to
  the underlying functionality.  Basically, it just reads in the input,
  simplifies it, and writes out the results.  It couldn't be simpler.

  Copyright (C) 1998 Michael Garland.  See "COPYING.txt" for details.
  
  $Id: qslim.cxx,v 1.10 2000/11/20 20:52:41 garland Exp $

 ************************************************************************/

#include <fstream>
#include <stdmix.h>
#include <MxTimer.h>
#include "qslim.h"

static ostream& vfcount(ostream& out, uint v, uint f)
{
    return out << "(" << v << "v/" << f << "f)";
}

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

void startup_and_input(int argc, char **argv)
{
    process_cmdline(argc, argv);
    if( m->face_count() == 0 )
    {
	smf->read(cin, m);
    }

    output_preamble();

#if (SAFETY > 0)
	if (!debug_stream)
	{
		if (output_filename)
			debug_stream = new ofstream(string(output_filename).append(".dbg"));
		else
			debug_stream = &cout;
	}

	mxmsg_set_handler(mxmsg_debug_handler);
	mxmsg_severity_level(MXMSG_TRACE);
#endif
}

int main(int argc, char **argv)
{
    double input_time, init_time, slim_time, output_time;
    bool result;

    // Process command line and read input model(s)
    //
    TIMING(input_time, startup_and_input(argc, argv));

    if(!be_quiet) cerr << "+ Initial model    ";
    if(!be_quiet) vfcount(cerr, m->vert_count(), m->face_count()) << endl;

    // Initial simplification process.  Collect contractions and build heap.
    //
    TIMING(init_time, slim_init());

    // Decimate model until target is reached
    //
    TIMING(slim_time, result = slim->decimate(face_target));
    if (!result)
    {
        while (history->length() > 0)
        {
            MxPairContraction& conx = history->drop();

            slim->apply_expansion(conx);

            if (slim->check_model())
                break;
        }
        if (history->length() == 0)
            return -1;
    }

    if(!be_quiet) cerr << "+ Simplified model ";
    if(!be_quiet) vfcount(cerr, slim->valid_verts, slim->valid_faces) << endl;

    // Output the result
    //
    TIMING(output_time, output_final_model());

    if( !be_quiet )
    {
	cerr << endl << endl;
	cerr << "+ Running time" << endl;
	cerr << "    Setup      : " << input_time << " sec" << endl;
	cerr << "    VDPMSlim init : " << init_time << " sec" << endl;
	cerr << "    VDPMSlim run  : " << slim_time << " sec" << endl;
	cerr << "    Output     : " << output_time << " sec" << endl;
	cerr << endl;
	cerr << "    Total      : "
	     << input_time+init_time+slim_time+output_time <<endl;
    }
    else
    {
	cerr << slim->valid_faces << " " << init_time+slim_time << endl;
    }

    slim_cleanup();

    return 0;
}
