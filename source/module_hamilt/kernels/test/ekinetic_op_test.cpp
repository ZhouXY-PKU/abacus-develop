#include <vector>
#include <complex>
#include <iostream>
#include <gtest/gtest.h>
#include "module_psi/kernels/memory_op.h"
#include "module_hamilt/kernels/ekinetic_op.h"

class TestModuleHamiltEkinetic : public ::testing::Test
{
  protected:
    // xx = tf.random.uniform([100], minval=-4, maxval=4, dtype = tf.float64)
    const int dim = 27;
    const int band = 8;
    const double tpiba2 = 0.37945422533;

    std::vector<double> gk2 = {
      3, 0, 3, 8, 3, 4, 4, 3, 8, 8, 3, 4, 4, 3, 8, 8, 8, 8, 8, 4, 3, 8, 8, 8, 8, 3, 4
    };

    std::vector<std::complex<double>> psi = {
      {0.140385,0}, {1.15637,0}, {0.140385,0}, {-0.00612284,-0}, {0.140385,0}, {0.0746255,0}, {0.0746255,0}, {0.140385,0}, {-0.00612284,-0}, {-0.00612284,-0}, {0.140385,0}, {0.0746255,0}, {0.0746255,0}, {0.140385,0}, {-0.00612284,-0}, {-0.00612284,-0}, {-0.00612284,-0}, {-0.00612284,-0}, {-0.00612284,-0}, {0.0746255,0}, {0.140385,0}, {-0.00612284,-0}, {-0.00612284,-0}, {-0.00612284,-0}, {-0.00612284,-0}, {0.140385,0}, {0.0746255,0},
      {-0,0.204435}, {0,-0}, {0,-0.204435}, {0,-0}, {0,-0.204435}, {0,-0.243733}, {-0,0.243733}, {-0,0.204435}, {0,-0}, {-0,0.0509928}, {-0,0.204435}, {0,-0}, {0,-0}, {0,-0.204435}, {0,-0.0509928}, {0,-0.0509928}, {-0,0.0509928}, {0,-0}, {0,-0}, {0,-0}, {0,-0.204435}, {0,-0.0509928}, {0,-0.0509928}, {-0,0.0509928}, {-0,0.0509928}, {-0,0.204435}, {0,-0},
      {-0,0.204435}, {-0,0}, {0,-0.204435}, {-0,0.0509928}, {-0,0.204435}, {-0,0}, {-0,0}, {0,-0.204435}, {0,-0.0509928}, {-0,0.0509928}, {-0,0.204435}, {-0,1.49244e-17}, {-0,0.243733}, {-0,0.204435}, {-0,3.12241e-18}, {-0,0.0509928}, {-0,3.12241e-18}, {-0,0.0509928}, {0,-0.0509928}, {-0,1.49244e-17}, {0,-0.204435}, {0,-0.0509928}, {-0,3.12241e-18}, {0,-0.0509928}, {-0,3.12241e-18}, {0,-0.204435}, {0,-0.243733},
      {0,-0.204435}, {-0,0}, {-0,0.204435}, {0,-0.0509928}, {0,-0.204435}, {-0,0}, {-0,0}, {-0,0.204435}, {-0,0.0509928}, {-0,0}, {-0,0.204435}, {-0,0.243733}, {-0,0}, {-0,0.204435}, {-0,0.0509928}, {-0,0}, {-0,0.0509928}, {-0,0.0509928}, {0,-0.0509928}, {0,-0.243733}, {0,-0.204435}, {-0,6.24482e-18}, {0,-0.0509928}, {-0,6.24482e-18}, {0,-0.0509928}, {0,-0.204435}, {-0,2.98487e-17},
      {8.59613e-18,0.140385}, {1.15637,0}, {8.59613e-18,-0.140385}, {-0.00612284,-0}, {8.59613e-18,-0.140385}, {-0.0746255,-9.13898e-18}, {-0.0746255,9.13898e-18}, {8.59613e-18,0.140385}, {-0.00612284,-0}, {-0.00612284,-0}, {8.59613e-18,-0.140385}, {-0.0746255,-9.13898e-18}, {-0.0746255,-9.13898e-18}, {-2.57884e-17,0.140385}, {-0.00612284,-1.49966e-18}, {-0.00612284,-1.49966e-18}, {-0.00612284,-0}, {-0.00612284,-1.49966e-18}, {-0.00612284,1.49966e-18}, {-0.0746255,9.13898e-18}, {8.59613e-18,0.140385}, {-0.00612284,-0}, {-0.00612284,-0}, {-0.00612284,1.49966e-18}, {-0.00612284,1.49966e-18}, {-2.57884e-17,-0.140385}, {-0.0746255,9.13898e-18},
      {-0.204435,1.2518e-17}, {0,-0}, {-0.204435,-1.2518e-17}, {0,-0}, {-0.204435,-1.2518e-17}, {-2.98487e-17,0.243733}, {-2.98487e-17,-0.243733}, {-0.204435,1.2518e-17}, {0,-0}, {-0,0.0509928}, {0.204435,1.2518e-17}, {-0,0}, {-0,0}, {0.204435,3.75541e-17}, {1.24896e-17,-0.0509928}, {1.24896e-17,-0.0509928}, {-0,0.0509928}, {0,-0}, {-0,-0}, {0,0}, {0.204435,-1.2518e-17}, {0,-0.0509928}, {0,-0.0509928}, {1.24896e-17,0.0509928}, {1.24896e-17,0.0509928}, {0.204435,-3.75541e-17}, {0,0},
      {-0.204435,1.2518e-17}, {-0,0}, {-0.204435,-1.2518e-17}, {-0,0.0509928}, {0.204435,1.2518e-17}, {0,-0}, {-0,-0}, {0.204435,-1.2518e-17}, {0,-0.0509928}, {-0,0.0509928}, {0.204435,1.2518e-17}, {1.82771e-33,-1.49244e-17}, {2.98487e-17,-0.243733}, {-0.204435,-3.75541e-17}, {-7.6477e-34,3.12241e-18}, {-1.24896e-17,0.0509928}, {-0,3.12241e-18}, {-1.24896e-17,0.0509928}, {-1.24896e-17,-0.0509928}, {-1.82771e-33,-1.49244e-17}, {0.204435,-1.2518e-17}, {0,-0.0509928}, {-0,3.12241e-18}, {-1.24896e-17,-0.0509928}, {7.6477e-34,3.12241e-18}, {-0.204435,3.75541e-17}, {2.98487e-17,0.243733},
      {0.204435,-1.2518e-17}, {-0,0}, {0.204435,1.2518e-17}, {0,-0.0509928}, {-0.204435,-1.2518e-17}, {0,-0}, {-0,-0}, {-0.204435,1.2518e-17}, {-0,0.0509928}, {-0,0}, {0.204435,1.2518e-17}, {2.98487e-17,-0.243733}, {0,-0}, {-0.204435,-3.75541e-17}, {-1.24896e-17,0.0509928}, {-0,0}, {-0,0.0509928}, {-1.24896e-17,0.0509928}, {-1.24896e-17,-0.0509928}, {2.98487e-17,0.243733}, {0.204435,-1.2518e-17}, {-0,6.24482e-18}, {0,-0.0509928}, {1.52954e-33,6.24482e-18}, {-1.24896e-17,-0.0509928}, {-0.204435,3.75541e-17}, {-3.65542e-33,-2.98487e-17}
    };

    std::vector<std::complex<double> > expected_hpsi = {
      {0.15981,0}, {0,0}, {0.15981,0}, {-0.0185867,0}, {0.15981,0}, {0.113268,0}, {0.113268,0}, {0.15981,0}, {-0.0185867,0}, {-0.0185867,0}, {0.15981,0}, {0.113268,0}, {0.113268,0}, {0.15981,0}, {-0.0185867,0}, {-0.0185867,0}, {-0.0185867,0}, {-0.0185867,0}, {-0.0185867,0}, {0.113268,0}, {0.15981,0}, {-0.0185867,0}, {-0.0185867,0}, {-0.0185867,0}, {-0.0185867,0}, {0.15981,0}, {0.113268,0},
      {0,0.232721}, {0,0}, {0,-0.232721}, {0,0}, {0,-0.232721}, {0,-0.369943}, {0,0.369943}, {0,0.232721}, {0,0}, {0,0.154796}, {0,0.232721}, {0,0}, {0,0}, {0,-0.232721}, {0,-0.154796}, {0,-0.154796}, {0,0.154796}, {0,0}, {0,0}, {0,0}, {0,-0.232721}, {0,-0.154796}, {0,-0.154796}, {0,0.154796}, {0,0.154796}, {0,0.232721}, {0,0},
      {0,0.232721}, {0,0}, {0,-0.232721}, {0,0.154796}, {0,0.232721}, {0,0}, {0,0}, {0,-0.232721}, {0,-0.154796}, {0,0.154796}, {0,0.232721}, {0,2.26525e-17}, {0,0.369943}, {0,0.232721}, {0,9.4785e-18}, {0,0.154796}, {0,9.4785e-18}, {0,0.154796}, {0,-0.154796}, {0,2.26525e-17}, {0,-0.232721}, {0,-0.154796}, {0,9.4785e-18}, {0,-0.154796}, {0,9.4785e-18}, {0,-0.232721}, {0,-0.369943},
      {0,-0.232721}, {0,0}, {0,0.232721}, {0,-0.154796}, {0,-0.232721}, {0,0}, {0,0}, {0,0.232721}, {0,0.154796}, {0,0}, {0,0.232721}, {0,0.369943}, {0,0}, {0,0.232721}, {0,0.154796}, {0,0}, {0,0.154796}, {0,0.154796}, {0,-0.154796}, {0,-0.369943}, {0,-0.232721}, {0,1.8957e-17}, {0,-0.154796}, {0,1.8957e-17}, {0,-0.154796}, {0,-0.232721}, {0,4.53049e-17},
      {9.78551e-18,0.15981}, {0,0}, {9.78551e-18,-0.15981}, {-0.0185867,0}, {9.78551e-18,-0.15981}, {-0.113268,-1.38713e-17}, {-0.113268,1.38713e-17}, {9.78551e-18,0.15981}, {-0.0185867,0}, {-0.0185867,0}, {9.78551e-18,-0.15981}, {-0.113268,-1.38713e-17}, {-0.113268,-1.38713e-17}, {-2.93565e-17,0.15981}, {-0.0185867,-4.55243e-18}, {-0.0185867,-4.55243e-18}, {-0.0185867,0}, {-0.0185867,-4.55243e-18}, {-0.0185867,4.55243e-18}, {-0.113268,1.38713e-17}, {9.78551e-18,0.15981}, {-0.0185867,0}, {-0.0185867,0}, {-0.0185867,4.55243e-18}, {-0.0185867,4.55243e-18}, {-2.93565e-17,-0.15981}, {-0.113268,1.38713e-17},
      {-0.232721,1.42501e-17}, {0,0}, {-0.232721,-1.42501e-17}, {0,0}, {-0.232721,-1.42501e-17}, {-4.53049e-17,0.369943}, {-4.53049e-17,-0.369943}, {-0.232721,1.42501e-17}, {0,0}, {0,0.154796}, {0.232721,1.42501e-17}, {0,0}, {0,0}, {0.232721,4.27502e-17}, {3.7914e-17,-0.154796}, {3.7914e-17,-0.154796}, {0,0.154796}, {0,0}, {0,0}, {0,0}, {0.232721,-1.42501e-17}, {0,-0.154796}, {0,-0.154796}, {3.7914e-17,0.154796}, {3.7914e-17,0.154796}, {0.232721,-4.27502e-17}, {0,0},
      {-0.232721,1.42501e-17}, {0,0}, {-0.232721,-1.42501e-17}, {0,0.154796}, {0.232721,1.42501e-17}, {0,0}, {0,0}, {0.232721,-1.42501e-17}, {0,-0.154796}, {0,0.154796}, {0.232721,1.42501e-17}, {2.77413e-33,-2.26525e-17}, {4.53049e-17,-0.369943}, {-0.232721,-4.27502e-17}, {-2.32156e-33,9.4785e-18}, {-3.7914e-17,0.154796}, {0,9.4785e-18}, {-3.7914e-17,0.154796}, {-3.7914e-17,-0.154796}, {-2.77413e-33,-2.26525e-17}, {0.232721,-1.42501e-17}, {0,-0.154796}, {0,9.4785e-18}, {-3.7914e-17,-0.154796}, {2.32156e-33,9.4785e-18}, {-0.232721,4.27502e-17}, {4.53049e-17,0.369943},
      {0.232721,-1.42501e-17}, {0,0}, {0.232721,1.42501e-17}, {0,-0.154796}, {-0.232721,-1.42501e-17}, {0,0}, {0,0}, {-0.232721,1.42501e-17}, {0,0.154796}, {0,0}, {0.232721,1.42501e-17}, {4.53049e-17,-0.369943}, {0,0}, {-0.232721,-4.27502e-17}, {-3.7914e-17,0.154796}, {0,0}, {0,0.154796}, {-3.7914e-17,0.154796}, {-3.7914e-17,-0.154796}, {4.53049e-17,0.369943}, {0.232721,-1.42501e-17}, {0,1.8957e-17}, {0,-0.154796}, {4.64312e-33,1.8957e-17}, {-3.7914e-17,-0.154796}, {-0.232721,4.27502e-17}, {-5.54825e-33,-4.53049e-17}
    };

    const psi::DEVICE_CPU * cpu_ctx = {};
    const psi::DEVICE_GPU * gpu_ctx = {};

    void SetUp() override {
    }
    void TearDown() override {
    }

    using ekinetic_cpu_op = hamilt::ekinetic_pw_op<double, psi::DEVICE_CPU>;
    using ekinetic_gpu_op = hamilt::ekinetic_pw_op<double, psi::DEVICE_GPU>;
    using set_memory_complex_double_op = psi::memory::set_memory_op<std::complex<double>, psi::DEVICE_GPU>;
    using delete_memory_double_op = psi::memory::delete_memory_op<double, psi::DEVICE_GPU>;
    using delete_memory_complex_double_op = psi::memory::delete_memory_op<std::complex<double>, psi::DEVICE_GPU>;
    using resize_memory_double_op = psi::memory::resize_memory_op<double, psi::DEVICE_GPU>;
    using resize_memory_complex_double_op = psi::memory::resize_memory_op<std::complex<double>, psi::DEVICE_GPU>;
    using syncmem_d_h2d_op = psi::memory::synchronize_memory_op<double, psi::DEVICE_GPU, psi::DEVICE_CPU>;
    using syncmem_cd_h2d_op = psi::memory::synchronize_memory_op<std::complex<double>, psi::DEVICE_GPU, psi::DEVICE_CPU>;
    using syncmem_cd_d2h_op = psi::memory::synchronize_memory_op<std::complex<double>, psi::DEVICE_CPU, psi::DEVICE_GPU>;
};

TEST_F(TestModuleHamiltEkinetic, ekinetic_pw_op_cpu)
{
  std::vector<std::complex<double> > hpsi(expected_hpsi.size(), std::complex<double>(0.0, 0.0));
  ekinetic_cpu_op()(cpu_ctx, band, dim, dim, tpiba2, gk2.data(), hpsi.data(), psi.data());
  for (int ii = 0; ii < hpsi.size(); ii++) {
    EXPECT_LT(std::abs(hpsi[ii] - expected_hpsi[ii]), 1e-6);
  }
}

#if __CUDA || __UT_USE_CUDA || __ROCM || __UT_USE_ROCM
TEST_F(TestModuleHamiltEkinetic, ekinetic_pw_op_gpu)
{
  double* gk2_dev = NULL;
  std::complex<double>* hpsi_dev = NULL, * psi_dev = NULL;
  resize_memory_double_op()(gpu_ctx, gk2_dev, gk2.size());
  resize_memory_complex_double_op()(gpu_ctx, psi_dev, psi.size());
  std::vector<std::complex<double> > hpsi(expected_hpsi.size(), std::complex<double>(0.0, 0.0));
  resize_memory_complex_double_op()(gpu_ctx, hpsi_dev, hpsi.size());
  syncmem_cd_h2d_op()(gpu_ctx, cpu_ctx, hpsi_dev, hpsi.data(), hpsi.size());
  syncmem_d_h2d_op()(gpu_ctx, cpu_ctx, gk2_dev, gk2.data(), gk2.size());
  syncmem_cd_h2d_op()(gpu_ctx, cpu_ctx, psi_dev, psi.data(), psi.size());
  // ekinetic_cpu_op()(cpu_ctx, band, dim, dim, tpiba2, gk2.data(), hpsi.data(), psi.data());
  ekinetic_gpu_op()(gpu_ctx, band, dim, dim, tpiba2, gk2_dev, hpsi_dev, psi_dev);
  syncmem_cd_d2h_op()(cpu_ctx, gpu_ctx, hpsi.data(), hpsi_dev, hpsi.size());

  for (int ii = 0; ii < hpsi.size(); ii++) {
    EXPECT_LT(fabs(hpsi[ii] - expected_hpsi[ii]), 1e-6);
  }
  delete_memory_double_op()(gpu_ctx, gk2_dev);
  delete_memory_complex_double_op()(gpu_ctx, psi_dev);
  delete_memory_complex_double_op()(gpu_ctx, hpsi_dev);
}
#endif // __CUDA || __UT_USE_CUDA || __ROCM || __UT_USE_ROCM