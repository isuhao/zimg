#ifdef ZIMG_X86

#include <cmath>
#include "common/cpuinfo.h"
#include "common/pixel.h"
#include "graph/image_filter.h"
#include "colorspace/colorspace.h"

#include "gtest/gtest.h"
#include "graph/filter_validator.h"

namespace {

void test_case(const zimg::colorspace::ColorspaceDefinition &csp_in, const zimg::colorspace::ColorspaceDefinition &csp_out,
			   const char * const expected_sha1[3], double expected_snr)
{
	const unsigned w = 640;
	const unsigned h = 480;

	if (!zimg::query_x86_capabilities().avx2) {
		SUCCEED() << "avx2 not available, skipping";
		return;
	}

	zimg::PixelFormat format = zimg::PixelType::FLOAT;
	auto builder = zimg::colorspace::ColorspaceConversion{ w, h }
		.set_csp_in(csp_in)
		.set_csp_out(csp_out)
		.set_approximate_gamma(true);

	auto filter_c = builder.set_cpu(zimg::CPUClass::NONE).create();
	auto filter_avx2 = builder.set_cpu(zimg::CPUClass::X86_AVX2).create();

	validate_filter(filter_avx2.get(), w, h, format, expected_sha1);
	validate_filter_reference(filter_avx2.get(), filter_c.get(), w, h, format, expected_snr);
}

} // namespace


TEST(ColorspaceConversionAVX2Test, test_transfer_lut)
{
	using namespace zimg::colorspace;

	const char *expected_sha1[][3] = {
		{
			"73f3a3c16120ec49271d9e5904071e3e622eca9b",
			"9ad3294130e4d8fbe68216e2a879d44d57a124c8",
			"182f73790a3e83005046ab165279e150d928c150"
		},
		{
			"8ec9a3b1697836107844745b36cb4e2fe4e237d1",
			"1ecba251a2cc9455b46de797f373c2a7f103a2ba",
			"017ab57a8734a97366e516a5b5f28d66ed47e397"
		},
		{
			"48c1ec7de50653817c678ea87ee6e1c84ef014d5",
			"58eb1dde0eb88fff043364836e1844aa766b64c5",
			"85a277a80dfca2e21789cedd76aaee307dbc4562"
		},
		{
			"905f2225e7b1cdf50225734a40e6198ddf99623f",
			"651f2aa9dd5dc52260b0a5a1ab353bda8c3abfc8",
			"d663cf53c3364bcee71d893f3dd868da05c0cb79"
		},
	};
	const double expected_tolinear_snr = 80.0;
	const double expected_togamma_snr = 80.0;

	SCOPED_TRACE("tolinear 709");
	test_case({ MatrixCoefficients::RGB, TransferCharacteristics::REC_709, ColorPrimaries::UNSPECIFIED },
	          { MatrixCoefficients::RGB, TransferCharacteristics::LINEAR, ColorPrimaries::UNSPECIFIED },
	          expected_sha1[0], expected_tolinear_snr);
	SCOPED_TRACE("togamma 709");
	test_case({ MatrixCoefficients::RGB, TransferCharacteristics::LINEAR, ColorPrimaries::UNSPECIFIED },
	          { MatrixCoefficients::RGB, TransferCharacteristics::REC_709, ColorPrimaries::UNSPECIFIED },
	          expected_sha1[1], expected_togamma_snr);
	SCOPED_TRACE("tolinear st2084");
	test_case({ MatrixCoefficients::RGB, TransferCharacteristics::ST_2084, ColorPrimaries::UNSPECIFIED },
	          { MatrixCoefficients::RGB, TransferCharacteristics::LINEAR, ColorPrimaries::UNSPECIFIED },
	          expected_sha1[2], expected_tolinear_snr);
	SCOPED_TRACE("togamma st2084");
	test_case({ MatrixCoefficients::RGB, TransferCharacteristics::LINEAR, ColorPrimaries::UNSPECIFIED },
	          { MatrixCoefficients::RGB, TransferCharacteristics::ST_2084, ColorPrimaries::UNSPECIFIED },
	          expected_sha1[3], expected_togamma_snr);
}

#endif // ZIMG_X86
