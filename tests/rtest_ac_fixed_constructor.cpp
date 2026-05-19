#include <ac_fixed.h>
#include <optional>
#include <string>

namespace AC_TYPES_TEST {

using namespace std;

namespace AC_DEBUG {

struct AC_DEBUG_INFO {
  std::optional<int> qb = 0;
  std::optional<int> r = 0;
  uint64_t check_rounded = 0;

  ~AC_DEBUG_INFO() = default;
};

string to_fixed_bin_str(uint64_t value, const int W, const int I) {
  bool bit;
  string bitstring;
  for (int i = 0; i < W; i++) {
    if (i == W - I)
      bitstring += ".";
    bit = value % 2;
    value >>= 1;
    bitstring += to_string(static_cast<int>(bit));
  }
  return string(bitstring.rbegin(), bitstring.rend());
}

} // namespace AC_DEBUG

struct TestResult {
  long long int total = 0;
  long long int failed = 0;

  TestResult() : total(0), failed(0) {}
  ~TestResult() = default;

  TestResult &operator+=(const TestResult &rhs) {
    total += rhs.total;
    failed += rhs.failed;
    return *this;
  }
};

bool quantization_adjust(uint64_t &base, const int outW, const ac_q_mode AC_Q,
                         bool qb, const bool r, const bool s) {
  bool carry = static_cast<bool>(base % (outW - 1));
  if (AC_Q == AC_TRN)
    return false;
  if (AC_Q == AC_RND_ZERO)
    qb &= s || r;
  else if (AC_Q == AC_RND_MIN_INF)
    qb &= r;
  else if (AC_Q == AC_RND_INF)
    qb &= !s || r;
  else if (AC_Q == AC_RND_CONV)
    qb &= (base & 1) || r;
  else if (AC_Q == AC_TRN_ZERO)
    qb = s && (qb || r);

  base += qb;
  return carry && (!static_cast<bool>(base >> (outW - 1)) & 1);
}

void saturation(double &value, ac_o_mode sat_mode) {
  if (sat_mode == AC_SAT) {
    if (value > 1 - pow(2, 0 - 4)) {
      value = 1 - pow(2, -4);
    }
  }
}

} // namespace AC_TYPES_TEST

struct AC_FIXED_INFO {
  const int W, I;
  const bool S;
  const ac_q_mode Q;
  const ac_o_mode O;

  AC_FIXED_INFO(const int AC_W, const int AC_I, const bool AC_S = true,
                const ac_q_mode AC_Q = AC_TRN, const ac_o_mode AC_O = AC_WRAP)
      : W(AC_W), I(AC_I), S(AC_S), Q(AC_Q), O(AC_O) {}
  ~AC_FIXED_INFO() = default;
};

#include <cmath>
#include <iostream>
using namespace std;

template <class T_in, class T_out>
bool output_check_signed(const T_in input, const T_out output,
                         const AC_FIXED_INFO &in_info,
                         const AC_FIXED_INFO &out_info) {
  bool correct = true;
  return correct;
}

template <class T_in, class T_out>
bool output_check_unsigned(const T_in input, const T_out output,
                           const AC_FIXED_INFO &in_info,
                           const AC_FIXED_INFO &out_info) {
  // TODO: fix negative numbers
  if (input < 0)
    return true;

  int F_out = out_info.W - out_info.I;
  int F_in = in_info.W - in_info.I;

#ifdef DEBUG
  using namespace AC_TYPES_TEST::AC_DEBUG;
  AC_DEBUG_INFO debug_info;
#endif

  uint64_t check_in = static_cast<uint64_t>(input.to_double() * pow(2, F_in));
  uint check;
  if (F_in > F_out) {
    if (out_info.Q != AC_TRN && !(out_info.Q == AC_TRN_ZERO && !in_info.S)) {
      // TODO: Should do checks
      bool qbit = (check_in >> (F_in - F_out - 1)) % 2;
      // TODO: Should add check for sign on quantization bit
      bool r =
          (check_in &
           (static_cast<uint64_t>(std::pow(2, F_in - F_out - 1)) - 1)) != 0;
      uint64_t check_rounded = check_in >> (F_in - F_out);
      AC_TYPES_TEST::quantization_adjust(
          check_rounded, out_info.W, out_info.Q, qbit, r,
          // Assuming in is always signed
          check_in & static_cast<uint64_t>(std::pow(2, in_info.W - 1)));
      check = check_rounded;

#ifdef DEBUG
      debug_info.qb = std::optional<int>(qbit);
      debug_info.r = std::optional<int>(r);
      debug_info.check_rounded = check_rounded;
#endif
    }
  }

  double check_pre_sat = check * pow(2, -F_out);
  double check_out = check_pre_sat;
  AC_TYPES_TEST::saturation(check_out, out_info.O);

  bool correct = check_out == output;
  correct =
      correct || (std::abs((double)output - check_out) < std::pow(10, -F_out));

#ifdef DEBUG
  if (!correct) {
    cout << endl;
    cout << "  Output not correct" << endl;
    cout << "  input  = " << input << endl;
    cout << "  input shifted (NOT ROUNDED) = "
         << to_fixed_bin_str(check_in, in_info.W, in_info.I) << "\n";
    if (debug_info.qb.has_value())
      cout << "  qb                          = " << debug_info.qb.value()
           << "\n";
    if (debug_info.r.has_value())
      cout << "  r                           = " << debug_info.r.value()
           << "\n";
    cout << "  input shifted (ROUNDED)     = "
         << to_fixed_bin_str(debug_info.check_rounded, out_info.W, out_info.I)
         << "\n"
         << "  input shifted and rounded   = " << check_pre_sat << "\n"
         << "  expected output             = " << check_out << endl;
    cout << "  output = " << output << endl;

    // assert(false);
  }
#endif

  return correct;
}

template <int inW, int inI, int outW, int outI, ac_q_mode outQ = AC_TRN,
          ac_o_mode outO = AC_WRAP>
struct AC_TYPES_TEST::TestResult test_ac_constructor(bool &all_tests_pass,
                                                     bool details = false) {
  AC_TYPES_TEST::TestResult res;

  ac_fixed<inW, inI> input_s_fixed;
  AC_FIXED_INFO in_info(inW, inI);
  ac_fixed<outW, outI, false, outQ, outO> output_us_fixed;
  AC_FIXED_INFO out_us_info(outW, outI, false, outQ, outO);
  ac_fixed<outW, outI, true, outQ, outO> output_s_fixed;
  AC_FIXED_INFO out_s_info(outW, outI, true, outQ, outO);

  cout << "TEST: ac_fixed constructor INPUT: ";
  cout.width(38);
  cout << left << input_s_fixed.type_name();
  cout << "OUTPUTS: ";
  cout.width(38);
  cout << left << output_us_fixed.type_name() << " ";
  cout << left << output_s_fixed.type_name() << endl;
  cout << "QUANT MODE: " << outQ << " " << "SAT MODE: " << outO << " ";
  cout.width(38);
  cout << "RESULT: ";

  double in_ll /*lower limit */, in_ul /*upper limit*/, in_step /*step*/,
      out_s_ll, out_s_ul, out_s_step, out_us_ll, out_us_ul, out_us_step;

  // set ranges and step size for integer testbench
  in_ll = input_s_fixed.template set_val<AC_VAL_MIN>().to_double();
  in_ul = input_s_fixed.template set_val<AC_VAL_MAX>().to_double();
  in_step = input_s_fixed.template set_val<AC_VAL_QUANTUM>().to_double();

  out_s_ll = output_s_fixed.template set_val<AC_VAL_MIN>().to_double();
  out_s_ul = output_s_fixed.template set_val<AC_VAL_MAX>().to_double();
  out_s_step = output_s_fixed.template set_val<AC_VAL_QUANTUM>().to_double();

  out_us_ll = output_us_fixed.template set_val<AC_VAL_MIN>().to_double();
  out_us_ul = output_us_fixed.template set_val<AC_VAL_MAX>().to_double();
  out_us_step = output_us_fixed.template set_val<AC_VAL_QUANTUM>().to_double();

  // Dump the test details
  if (details) {
    cout << endl;                                          // LCOV_EXCL_LINE
    cout << "  Ranges for input types:" << endl;           // LCOV_EXCL_LINE
    cout << "    lower_limit = " << in_ll << endl;         // LCOV_EXCL_LINE
    cout << "    upper_limit = " << in_ul << endl;         // LCOV_EXCL_LINE
    cout << "    step        = " << in_step << endl;       // LCOV_EXCL_LINE
    cout << endl;                                          // LCOV_EXCL_LINE
    cout << "  Ranges for output types:" << endl;          // LCOV_EXCL_LINE
    cout << "    SIGNED:" << endl;                         // LCOV_EXCL_LINE
    cout << "      lower_limit = " << out_s_ll << endl;    // LCOV_EXCL_LINE
    cout << "      upper_limit = " << out_s_ul << endl;    // LCOV_EXCL_LINE
    cout << "      step        = " << out_s_step << endl;  // LCOV_EXCL_LINE
    cout << "    UNSIGNED:" << endl;                       // LCOV_EXCL_LINE
    cout << "      lower_limit = " << out_us_ll << endl;   // LCOV_EXCL_LINE
    cout << "      upper_limit = " << out_us_ul << endl;   // LCOV_EXCL_LINE
    cout << "      step        = " << out_us_step << endl; // LCOV_EXCL_LINE
  }

  bool correct = true;

  for (double i = in_ll; i <= in_ul; i += in_step) {
    input_s_fixed = i;
    output_s_fixed = input_s_fixed;
    output_us_fixed = input_s_fixed;

    bool correct_iteration =
        output_check_signed(input_s_fixed, output_s_fixed, in_info,
                            out_s_info) &&
        output_check_unsigned(input_s_fixed, output_us_fixed, in_info,
                              out_us_info);
    correct = correct && correct_iteration;
    res.total++;
    if (!correct_iteration)
      res.failed++;
  }

  if (correct) {
    cout << "PASSED" << endl;
  } else {
    cout << "FAILED " << res.failed << "/" << res.total << endl;
  }

  all_tests_pass = all_tests_pass && correct;

  return res;
}

template <int inW, int inI, int outW, int outI, ac_q_mode outQ = AC_TRN,
          ac_o_mode outO = AC_WRAP>
struct AC_TYPES_TEST::TestResult test_ac_constructor(bool &all_tests_pass,
                                                     double init_value,
                                                     bool details = false) {
  AC_TYPES_TEST::TestResult res;

  ac_fixed<inW, inI> input_s_fixed;
  AC_FIXED_INFO in_info(inW, inI);
  ac_fixed<outW, outI, false, outQ, outO> output_us_fixed;
  AC_FIXED_INFO out_us_info(outW, outI, false, outQ, outO);
  ac_fixed<outW, outI, true, outQ, outO> output_s_fixed;
  AC_FIXED_INFO out_s_info(outW, outI, true, outQ, outO);

  cout << "TEST: ac_fixed constructor INPUT: ";
  cout.width(38);
  cout << left << input_s_fixed.type_name();
  cout << "OUTPUTS: ";
  cout.width(38);
  cout << left << output_us_fixed.type_name() << " ";
  cout << left << output_s_fixed.type_name() << endl;
  cout << "QUANT MODE: " << outQ << " " << "SAT MODE: " << outO << " ";
  cout.width(38);
  cout << "RESULT: ";

  bool correct = true;

  input_s_fixed = init_value;
  output_s_fixed = input_s_fixed;
  output_us_fixed = input_s_fixed;

  correct =
      output_check_signed(input_s_fixed, output_s_fixed, in_info, out_s_info) &&
      output_check_unsigned(input_s_fixed, output_us_fixed, in_info,
                            out_us_info);
  res.total++;
  if (!correct)
    res.failed++;

  if (correct) {
    cout << "PASSED" << endl;
  } else {
    cout << "FAILED " << res.failed << "/" << res.total << endl;
  }

  all_tests_pass = all_tests_pass && correct;

  return res;
}

template <int W, int I, ac_q_mode Q = AC_TRN, ac_o_mode O = AC_WRAP>
struct AC_TYPES_TEST::TestResult test_ac_assignment(bool &all_tests_pass,
                                                    double init_value) {
  AC_TYPES_TEST::TestResult res;

  ac_fixed<W, I> input_s_fixed;
  AC_FIXED_INFO in_info(W, I);
  ac_fixed<W, I, false, Q, O> output_us_fixed;
  AC_FIXED_INFO out_us_info(W, I, false, Q, O);
  ac_fixed<W, I, true, Q, O> output_s_fixed;
  AC_FIXED_INFO out_s_info(W, I, true, Q, O);

  cout << "TEST: ac_fixed constructor INPUT: ";
  cout.width(38);
  cout << left << init_value;
  cout << "OUTPUTS: ";
  cout.width(38);
  cout << left << output_s_fixed.type_name();
  cout << left << output_us_fixed.type_name();
  cout << "RESULT: ";

  bool correct =
      output_check_signed(input_s_fixed, output_s_fixed, in_info, out_s_info) &&
      output_check_unsigned(input_s_fixed, output_us_fixed, in_info,
                            out_us_info);

  res.total++;
  if (!correct)
    res.failed++;

  if (correct) {
    cout << "PASSED" << endl;
  } else {
    cout << "FAILED " << res.failed << "/" << res.total << endl;
  }

  all_tests_pass = all_tests_pass && correct;

  return res;
}

int main(int argc, char *argv[]) {
  cout << "===================================================================="
          "========="
       << endl;
  cout << "Testing ac_fixed constructor" << endl;

  bool all_tests_pass = true;

  // If any of the tests fail, the all_tests_pass variable will be set to false
  AC_TYPES_TEST::TestResult res;

#ifdef MANUAL
#else
  res += test_ac_constructor<16, 6, 4, 0, AC_RND_CONV, AC_SAT>(all_tests_pass,
                                                               true);

  // ---- QUANTIZATION (Fin > Fout) ----
  res += test_ac_constructor<16, 6, 4, 0, AC_RND>(all_tests_pass, true);
  res += test_ac_constructor<16, 6, 4, 0, AC_RND_ZERO>(all_tests_pass, true);
  res += test_ac_constructor<16, 6, 4, 0, AC_RND_MIN_INF>(all_tests_pass, true);
  res += test_ac_constructor<16, 6, 4, 0, AC_RND_INF>(all_tests_pass, true);
  res += test_ac_constructor<16, 6, 4, 0, AC_RND_CONV>(all_tests_pass, true);
  res += test_ac_constructor<16, 6, 4, 0, AC_TRN>(all_tests_pass, true);
  res += test_ac_constructor<16, 6, 4, 0, AC_TRN_ZERO>(all_tests_pass, true);

  // ---- EDGE CASES (Fin > Fout) ----
  res += test_ac_constructor<8, 0, 3, 3, AC_RND_CONV>(all_tests_pass, true);
  res += test_ac_constructor<4, 0, 10, 10, AC_RND_CONV>(all_tests_pass, true);

  // ---- EDGE CASES (Fin < Fout) ----
  res += test_ac_constructor<8, 5, 4, 0, AC_RND_ZERO, AC_SAT_SYM>(
      all_tests_pass, true);
  res += test_ac_constructor<8, 5, 10, 0, AC_RND_ZERO, AC_SAT_SYM>(
      all_tests_pass, true);

  // ---- QUANTIZATION ON ASSIGNMENT (double -> ac_fixed) ----

  // ap_fixed<3, 2, AP_RND, AP_SAT> UAPFixed4 = 1.25; // Yields: 1.5
  // ap_fixed<3, 2, AP_RND, AP_SAT> UAPFixed4 = -1.25; // Yields: -1.0
  res += test_ac_assignment<3, 2, AC_RND, AC_SAT>(all_tests_pass, 1.25);
  res += test_ac_assignment<3, 2, AC_RND, AC_SAT>(all_tests_pass, -1.25);

  // ap_fixed<3, 2, AP_RND_ZERO, AP_SAT> UAPFixed4 = 1.25; // Yields: 1.0
  // ap_fixed<3, 2, AP_RND_ZERO, AP_SAT> UAPFixed4 = -1.25; // Yields: -1.0
  res += test_ac_assignment<3, 2, AC_RND_ZERO, AC_SAT>(all_tests_pass, 1.25);
  res += test_ac_assignment<3, 2, AC_RND_ZERO, AC_SAT>(all_tests_pass, -1.25);

  // ap_fixed<3, 2, AP_RND_MIN_INF, AP_SAT> UAPFixed4 = 1.25; // Yields: 1.0
  // ap_fixed<3, 2, AP_RND_MIN_INF, AP_SAT> UAPFixed4 = -1.25; // Yields: -1.5
  res += test_ac_assignment<3, 2, AC_RND_MIN_INF, AC_SAT>(all_tests_pass, 1.25);
  res +=
      test_ac_assignment<3, 2, AC_RND_MIN_INF, AC_SAT>(all_tests_pass, -1.25);

  // ap_fixed<3, 2, AP_RND_INF, AP_SAT> UAPFixed4 = 1.25; // Yields: 1.5
  // ap_fixed<3, 2, AP_RND_INF, AP_SAT> UAPFixed4 = -1.25; // Yields: -1.5
  res += test_ac_assignment<3, 2, AC_RND_INF, AC_SAT>(all_tests_pass, 1.25);
  res += test_ac_assignment<3, 2, AC_RND_INF, AC_SAT>(all_tests_pass, -1.25);

  // ap_fixed<8,3> p1 = 1.59375; // p1 = 001.10011
  // ap_fixed<5,3,AP_RND_CONV> rconv1 = p1; // rconv1 = 1.5 (001.10)
  res += test_ac_constructor<8, 3, 5, 3, AC_RND_CONV>(all_tests_pass, 1.59375);
  res += test_ac_constructor<8, 3, 5, 3, AC_RND_CONV>(all_tests_pass, -1.59375);

  // ap_fixed<8,3> p2 = 1.625; // p2 = 001.10100 => tie with bit3 (LSB-to-be) =
  // 0 ap_fixed<5,3,AP_RND_CONV> rconv2 = p2; // rconv2 = 1.5 (001.10) => lsb is
  // already zero, just trunca
  res += test_ac_constructor<8, 3, 5, 3, AC_RND_CONV>(all_tests_pass, 1.625);
  res += test_ac_constructor<8, 3, 5, 3, AC_RND_CONV>(all_tests_pass, -1.625);

  // ap_fixed<8,3> p3 = 1.375; // p3 = 001.01100 => tie with bit3 (LSB-to-be) =
  // 1 ap_fixed<5,3,AP_RND_CONV> rconv3 = p3; // rconv3 = 1.5 (001.10) => lsb is
  // made zero by rounding up
  res += test_ac_constructor<8, 3, 5, 3, AC_RND_CONV>(all_tests_pass, 1.375);
  res += test_ac_constructor<8, 3, 5, 3, AC_RND_CONV>(all_tests_pass, -1.375);

  // ap_fixed<8,3> p3 = 1.65625; // p3 = 001.10101
  // ap_fixed<5,3,AP_RND_CONV> rconv3 = p3; // rconv3 = 1.75 (001.11) => round
  res += test_ac_constructor<8, 3, 5, 3, AC_RND_CONV>(all_tests_pass, 1.65625);
  res += test_ac_constructor<8, 3, 5, 3, AC_RND_CONV>(all_tests_pass, -1.65625);

  // ---- OVERFLOW ON ASSIGNMENT (double -> ac_fixed) ----
  // ---- OVERFLOW ON ASSIGNMENT (ac_fixed -> ac_fixed) ----

  // ap_fixed<3, 2, AP_TRN, AP_SAT> UAPFixed4 = 1.25; // Yields: 1.0
  // ap_fixed<3, 2, AP_TRN, AP_SAT> UAPFixed4 = -1.25; // Yields: -1.5
  res += test_ac_assignment<3, 2, AC_TRN, AC_SAT>(all_tests_pass, 1.25);
  res += test_ac_assignment<3, 2, AC_TRN, AC_SAT>(all_tests_pass, -1.25);

  // ap_fixed<3, 2, AP_TRN_ZERO, AP_SAT> UAPFixed4 = 1.25; // Yields: 1.0
  // ap_fixed<3, 2, AP_TRN_ZERO, AP_SAT> UAPFixed4 = -1.25; // Yields: -1.0
  res += test_ac_assignment<3, 2, AC_TRN_ZERO, AC_SAT>(all_tests_pass, 1.25);
  res += test_ac_assignment<3, 2, AC_TRN_ZERO, AC_SAT>(all_tests_pass, -1.25);

  // ap_fixed<4, 4, AP_RND, AP_SAT> UAPFixed4 = 19.0; // Yields: 7.0
  // ap_fixed<4, 4, AP_RND, AP_SAT> UAPFixed4 = -19.0; // Yields: -8.0
  // ap_ufixed<4, 4, AP_RND, AP_SAT> UAPFixed4 = 19.0; // Yields: 15.0
  // ap_ufixed<4, 4, AP_RND, AP_SAT> UAPFixed4 = -19.0; // Yields: 0.0
  res += test_ac_assignment<4, 4, AC_RND, AC_SAT>(all_tests_pass, 19.0);
  res += test_ac_assignment<4, 4, AC_RND, AC_SAT>(all_tests_pass, -19.0);

  // ap_fixed<4, 4, AP_RND, AP_SAT_ZERO> UAPFixed4 = 19.0; // Yields: 0.0
  // ap_fixed<4, 4, AP_RND, AP_SAT_ZERO> UAPFixed4 = -19.0; // Yields: 0.0
  // ap_ufixed<4, 4, AP_RND, AP_SAT_ZERO> UAPFixed4 = 19.0; // Yields: 0.0
  // ap_ufixed<4, 4, AP_RND, AP_SAT_ZERO> UAPFixed4 = -19.0; // Yields: 0.0
  res += test_ac_assignment<4, 4, AC_RND, AC_SAT_ZERO>(all_tests_pass, 19.0);
  res += test_ac_assignment<4, 4, AC_RND, AC_SAT_ZERO>(all_tests_pass, -19.0);

  // ap_fixed<4, 4, AP_RND, AP_SAT_SYM> UAPFixed4 = 19.0; // Yields: 7.0
  // ap_fixed<4, 4, AP_RND, AP_SAT_SYM> UAPFixed4 = -19.0; // Yields: -7.0
  // ap_ufixed<4, 4, AP_RND, AP_SAT_SYM> UAPFixed4 = 19.0; // Yields: 15.0
  // ap_ufixed<4, 4, AP_RND, AP_SAT_SYM> UAPFixed4 = -19.0; // Yields: 0.0
  res += test_ac_assignment<4, 4, AC_RND, AC_SAT_SYM>(all_tests_pass, 19.0);
  res += test_ac_assignment<4, 4, AC_RND, AC_SAT_SYM>(all_tests_pass, -19.0);

  // ap_fixed<4, 4, AP_RND, AP_WRAP> UAPFixed4 = 31.0; // Yields: -1.0
  // ap_fixed<4, 4, AP_RND, AP_WRAP> UAPFixed4 = -19.0; // Yields: -3.0
  // ap_ufixed<4, 4, AP_RND, AP_WRAP> UAPFixed4 = 19.0; // Yields: 3.0
  // ap_ufixed<4, 4, AP_RND, AP_WRAP> UAPFixed4 = -19.0; // Yields: 13.0
  res += test_ac_assignment<4, 4, AC_RND, AC_WRAP>(all_tests_pass, 31.0);
  res += test_ac_assignment<4, 4, AC_RND, AC_WRAP>(all_tests_pass, 19.0);
  res += test_ac_assignment<4, 4, AC_RND, AC_WRAP>(all_tests_pass, -19.0);
#endif

  cout << "===================================================================="
          "========="
       << endl;
  cout << "  Testbench finished." << endl;

  // Notify the user whether or not the test was a failure.
  if (!all_tests_pass) {
    cout << "  ac_fixed_constuctor - FAILED - Output not correct for "
         << res.failed << "/" << res.total << endl; // LCOV_EXCL_LINE
    cout << "=================================================================="
            "==========="
         << endl; // LCOV_EXCL_LINE
    return -1;    // LCOV_EXCL_LINE
  } else {
    cout << "  ac_fixed_constructor - PASSED" << endl;
    cout << "=================================================================="
            "==========="
         << endl;
  }

  return 0;
}
