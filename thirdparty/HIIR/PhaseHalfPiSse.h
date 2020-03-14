/*****************************************************************************

        PhaseHalfPiSse.h
        Author: Laurent de Soras, 2005

From the input signal, generates two signals with a pi/2 phase shift, using
SSE instruction set.

This object must be aligned on a 16-byte boundary!

If the number of coefficients is 2 or 3 modulo 4, the output is delayed from
1 sample, compared to the theoretical formula (or FPU implementation).

Template parameters:
	- NC: number of coefficients, > 0

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#if ! defined (hiir_PhaseHalfPiSse_HEADER_INCLUDED)
#define hiir_PhaseHalfPiSse_HEADER_INCLUDED

#if defined (_MSC_VER)
	#pragma once
	#pragma warning (4 : 4250) // "Inherits via dominance."
#endif



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "hiir/def.h"
#include "hiir/StageDataSse.h"

#include <xmmintrin.h>

#include <array>



namespace hiir
{



template <int NC>
class PhaseHalfPiSse
{

	static_assert ((NC > 0), "Number of coefficient must be positive.");

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

	typedef float DataType;
	static const int  _nbr_chn = 1;

	enum {         NBR_COEFS = NC };

	               PhaseHalfPiSse ();
	               PhaseHalfPiSse (const PhaseHalfPiSse <NC> &other) = default;
	               PhaseHalfPiSse (PhaseHalfPiSse <NC> &&other)      = default;
	               ~PhaseHalfPiSse ()                                = default;

	PhaseHalfPiSse <NC> &
	               operator = (const PhaseHalfPiSse <NC> &other)     = default;
	PhaseHalfPiSse <NC> &
	               operator = (PhaseHalfPiSse <NC> &&other)          = default;

	void           set_coefs (const double coef_arr []);

	hiir_FORCEINLINE void
	               process_sample (float &out_0, float &out_1, float input);
	void           process_block (float out_0_ptr [], float out_1_ptr [], const float in_ptr [], long nbr_spl);

	void           clear_buffers ();



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	enum {         STAGE_WIDTH = 4 };
	enum {         NBR_STAGES  = (NBR_COEFS + STAGE_WIDTH-1) / STAGE_WIDTH };
	enum {         NBR_PHASES  = 2 };

	typedef std::array <StageDataSse, NBR_STAGES + 1> Filter;   // Stage 0 contains only input memory
   typedef std::array <Filter, NBR_PHASES> FilterBiPhase;

	FilterBiPhase  _filter; // Should be the first member (thus easier to align)
	float          _prev;
	int            _phase;  // 0 or 1



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	bool           operator == (const PhaseHalfPiSse <NC> &other) = delete;
	bool           operator != (const PhaseHalfPiSse <NC> &other) = delete;

}; // class PhaseHalfPiSse



}  // namespace hiir



#include "hiir/PhaseHalfPiSse.hpp"



#endif   // hiir_PhaseHalfPiSse_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
