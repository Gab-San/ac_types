// TODO: Main stream to ac_tests:
// - #include <ac_types/ac_fixed.h>

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

#include <ac_fixed.h>
#include <ap_fixed.h>
#include <cmath>

constexpr ap_q_mode ac_to_ap_q_mode(const ac_q_mode Q) {
  switch (Q) {
  case AC_TRN:
    return AP_TRN;
  case AC_TRN_ZERO:
    return AP_TRN_ZERO;
  case AC_RND:
    return AP_RND;
  case AC_RND_ZERO:
    return AP_RND_ZERO;
  case AC_RND_MIN_INF:
    return AP_RND_MIN_INF;
  case AC_RND_INF:
    return AP_RND_INF;
  case AC_RND_CONV:
    return AP_RND_CONV;
  case AC_RND_CONV_ODD:
    return AP_RND_CONV;
  }
}
constexpr ap_o_mode ac_to_ap_o_mode(const ac_o_mode O) {
  switch (O) {
  case AC_WRAP:
    return AP_WRAP;
  case AC_SAT:
    return AP_SAT;
  case AC_SAT_SYM:
    return AP_SAT_SYM;
  case AC_SAT_ZERO:
    return AP_SAT_ZERO;
  }
}

#include <iostream>
using namespace std;

template <int Wout, int Iout, ap_q_mode Qout, ap_o_mode Oout, class T_in,
          class T_out>
bool output_check_signed(const T_in input, const T_out output) {
  ap_fixed<Wout, Iout, Qout, Oout> check = input.to_double();

  bool correct = check.to_double() == output.to_double();
  correct = correct || (abs(output.to_double() - check.to_double()) <
                        pow(10, -(Wout - Iout)));

#ifdef DEBUG
  if (!correct) {
    cout << endl;
    cout << "[SIGNED] Output not correct" << endl;
    cout << "  input           = " << input.to_string(AC_DEC) << endl;
    cout << "  expected output = " << check << endl;
    cout << "  output          = " << output.to_string(AC_DEC) << endl;

    // assert(false);
  }
#endif

  return correct;
}

template <int Wout, int Iout, ap_q_mode Qout, ap_o_mode Oout, class T_in,
          class T_out>
bool output_check_unsigned(const T_in input, const T_out output) {
  ap_ufixed<Wout, Iout, Qout, Oout> check = input.to_double();

  bool correct = check.to_double() == output.to_double();
  correct = correct || (abs(output.to_double() - check.to_double()) <
                        pow(10, -(Wout - Iout)));

#ifdef DEBUG
  if (!correct) {
    cout << endl;
    cout << "[UNSIGNED] Output not correct" << endl;
    cout << "  input           = " << input.to_string(AC_DEC) << endl;
    cout << "  expected output = " << check << endl;
    cout << "  output          = " << output.to_string(AC_DEC) << endl;

    // assert(false);
  }
#endif

  return correct;
}

template <int inW, int inI, int Wout, int Iout, ac_q_mode Qout = AC_TRN,
          ac_o_mode Oout = AC_WRAP>
struct TestResult test_ac_constructor(bool &all_tests_pass,
                                      bool details = false) {
  TestResult res;

  ac_fixed<inW, inI> input_s_fixed;
  ac_fixed<Wout, Iout, false, Qout, Oout> output_us_fixed;
  ac_fixed<Wout, Iout, true, Qout, Oout> output_s_fixed;

  cout << "TEST: ac_fixed constructor INPUT: ";
  cout.width(38);
  cout << left << input_s_fixed.type_name();
  cout << "OUTPUTS: ";
  cout.width(38);
  cout << left << output_s_fixed.type_name();
  cout << left << output_us_fixed.type_name();
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
    cout << endl;                                    // LCOV_EXCL_LINE
    cout << "  Ranges for input types:" << endl;     // LCOV_EXCL_LINE
    cout << "    lower_limit = " << in_ll << endl;   // LCOV_EXCL_LINE
    cout << "    upper_limit = " << in_ul << endl;   // LCOV_EXCL_LINE
    cout << "    step        = " << in_step << endl; // LCOV_EXCL_LINE
    cout << endl;                                    // LCOV_EXCL_LINE
#ifdef DEBUG
    cout << "  Ranges for output types:" << endl;          // LCOV_EXCL_LINE
    cout << "    SIGNED:" << endl;                         // LCOV_EXCL_LINE
    cout << "      lower_limit = " << out_s_ll << endl;    // LCOV_EXCL_LINE
    cout << "      upper_limit = " << out_s_ul << endl;    // LCOV_EXCL_LINE
    cout << "      step        = " << out_s_step << endl;  // LCOV_EXCL_LINE
    cout << "    UNSIGNED:" << endl;                       // LCOV_EXCL_LINE
    cout << "      lower_limit = " << out_us_ll << endl;   // LCOV_EXCL_LINE
    cout << "      upper_limit = " << out_us_ul << endl;   // LCOV_EXCL_LINE
    cout << "      step        = " << out_us_step << endl; // LCOV_EXCL_LINE
#endif
  }

  bool correct = true;

  for (double i = in_ll; i <= in_ul; i += in_step) {
    input_s_fixed = i;
    output_s_fixed = input_s_fixed;
    output_us_fixed = input_s_fixed;

    constexpr ap_q_mode AP_Q_OUT = ac_to_ap_q_mode(Qout);
    constexpr ap_o_mode AP_O_OUT = ac_to_ap_o_mode(Oout);

    bool correct_iteration =
        output_check_signed<Wout, Iout, AP_Q_OUT, AP_O_OUT>(input_s_fixed,
                                                            output_s_fixed) &&
        output_check_unsigned<Wout, Iout, AP_Q_OUT, AP_O_OUT>(input_s_fixed,
                                                              output_us_fixed);
    correct = correct && correct_iteration;
    res.total++;
    if (!correct_iteration)
      res.failed++;
  }

  if (correct) {
    cout << "PASSED" << endl;
    res.total = 1;
  } else {
    cout << "FAILED " << res.failed << "/" << res.total << endl;
    res.total = 1;
    res.failed = 1;
  }

  all_tests_pass = all_tests_pass && correct;

  return res;
}

template <int inW, int inI, int Wout, int Iout, ac_q_mode Qout = AC_TRN,
          ac_o_mode Oout = AC_WRAP>
struct TestResult test_ac_constructor(bool &all_tests_pass, double init_value) {
  TestResult res;

  ac_fixed<inW, inI> input_s_fixed;
  ac_fixed<Wout, Iout, false, Qout, Oout> output_us_fixed;
  ac_fixed<Wout, Iout, true, Qout, Oout> output_s_fixed;

  cout << "TEST: ac_fixed constructor INPUT: ";
  cout.width(38);
  cout << left << input_s_fixed.type_name();
  cout << "OUTPUTS: ";
  cout.width(38);
  cout << left << output_s_fixed.type_name();
  cout << left << output_us_fixed.type_name();
  cout << "RESULT: ";

  input_s_fixed = init_value;
  output_s_fixed = input_s_fixed;
  output_us_fixed = input_s_fixed;

  constexpr ap_q_mode AP_Q_OUT = ac_to_ap_q_mode(Qout);
  constexpr ap_o_mode AP_O_OUT = ac_to_ap_o_mode(Oout);

  bool correct = output_check_signed<Wout, Iout, AP_Q_OUT, AP_O_OUT>(
                     input_s_fixed, output_s_fixed) &&
                 output_check_unsigned<Wout, Iout, AP_Q_OUT, AP_O_OUT>(
                     input_s_fixed, output_us_fixed);
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
struct TestResult test_ac_assignment(bool &all_tests_pass, double init_value) {
  TestResult res;

  ac_fixed<W, I, true, Q, O> input_s_fixed = init_value;
  ac_fixed<W, I, false, Q, O> output_us_fixed = input_s_fixed;
  ac_fixed<W, I, true, Q, O> output_s_fixed = input_s_fixed;

  cout << "TEST: ac_fixed constructor INPUT: ";
  cout.width(38);
  cout << left << init_value;
  cout << "OUTPUTS: ";
  cout.width(38);
  cout << left << output_s_fixed.type_name();
  cout << left << output_us_fixed.type_name();
  cout << "RESULT: ";

  constexpr ap_q_mode AP_Q = ac_to_ap_q_mode(Q);
  constexpr ap_o_mode AP_O = ac_to_ap_o_mode(O);

  bool correct =
      output_check_signed<W, I, AP_Q, AP_O>(input_s_fixed, output_s_fixed) &&
      output_check_unsigned<W, I, AP_Q, AP_O>(input_s_fixed, output_us_fixed);

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
  TestResult res;

#ifdef MANUAL
  cout << "|MANUAL MODE|" << endl;
  res += test_ac_assignment<3, 2, AC_TRN, AC_SAT>(all_tests_pass, -1.25);
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
