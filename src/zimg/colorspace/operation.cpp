#include "common/except.h"
#include "common/zassert.h"
#include "colorspace.h"
#include "colorspace_param.h"
#include "operation.h"
#include "operation_impl.h"
#include "operation_impl_x86.h"

namespace zimg {
namespace colorspace {

Operation::~Operation() = default;

std::unique_ptr<Operation> create_ncl_yuv_to_rgb_operation(MatrixCoefficients matrix, const OperationParams &params, CPUClass cpu)
{
	return create_matrix_operation(ncl_yuv_to_rgb_matrix(matrix), cpu);
}

std::unique_ptr<Operation> create_ncl_rgb_to_yuv_operation(MatrixCoefficients matrix, const OperationParams &params, CPUClass cpu)
{
	return create_matrix_operation(ncl_rgb_to_yuv_matrix(matrix), cpu);
}

std::unique_ptr<Operation> create_gamma_to_linear_operation(TransferCharacteristics transfer, const OperationParams &params, CPUClass cpu)
{
	zassert_d(transfer != TransferCharacteristics::LINEAR, "linear op");

#ifdef ZIMG_X86
	if (std::unique_ptr<Operation> op = create_gamma_to_linear_operation_x86(transfer, params, cpu))
		return op;
#endif

	switch (transfer) {
	case TransferCharacteristics::REC_709:
		return create_rec709_inverse_gamma_operation(cpu);
	case TransferCharacteristics::SRGB:
		return create_srgb_inverse_gamma_operation(cpu);
	case TransferCharacteristics::ST_2084:
		return create_st2084_inverse_gamma_operation(params.peak_luminance, cpu);
	case TransferCharacteristics::ARIB_B67:
		return create_b67_inverse_gamma_operation(cpu);
	default:
		throw error::InternalError{ "unsupported transfer function" };
	}
}

std::unique_ptr<Operation> create_linear_to_gamma_operation(TransferCharacteristics transfer, const OperationParams &params, CPUClass cpu)
{
	zassert_d(transfer != TransferCharacteristics::LINEAR, "linear op");

#ifdef ZIMG_X86
	if (std::unique_ptr<Operation> op = create_linear_to_gamma_operation_x86(transfer, params, cpu))
		return op;
#endif

	switch (transfer) {
	case TransferCharacteristics::REC_709:
		return create_rec709_gamma_operation(cpu);
	case TransferCharacteristics::SRGB:
		return create_srgb_gamma_operation(cpu);
	case TransferCharacteristics::ST_2084:
		return create_st2084_gamma_operation(params.peak_luminance, cpu);
	case TransferCharacteristics::ARIB_B67:
		return create_b67_gamma_operation(cpu);
	default:
		throw error::InternalError{ "unsupported transfer function" };
	}
}

std::unique_ptr<Operation> create_gamut_operation(ColorPrimaries primaries_in, ColorPrimaries primaries_out, const OperationParams &params, CPUClass cpu)
{
	return create_matrix_operation(gamut_xyz_to_rgb_matrix(primaries_out) * gamut_rgb_to_xyz_matrix(primaries_in), cpu);
}

} // namespace colorspace
} // namespace zimg
