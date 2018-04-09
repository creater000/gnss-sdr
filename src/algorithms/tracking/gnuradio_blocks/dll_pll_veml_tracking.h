/*!
 * \file dll_pll_veml_tracking.h
 * \brief Implementation of a code DLL + carrier PLL tracking block.
 * \author Antonio Ramos, 2018 antonio.ramosdet(at)gmail.com
 *
 * -------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018  (see AUTHORS file for a list of contributors)
 *
 * GNSS-SDR is a software defined Global Navigation
 *          Satellite Systems receiver
 *
 * This file is part of GNSS-SDR.
 *
 * GNSS-SDR is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GNSS-SDR is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNSS-SDR. If not, see <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------
 */

#ifndef GNSS_SDR_DLL_PLL_VEML_TRACKING_H
#define GNSS_SDR_DLL_PLL_VEML_TRACKING_H

#include "gnss_synchro.h"
#include "tracking_2nd_DLL_filter.h"
#include "tracking_2nd_PLL_filter.h"
#include "cpu_multicorrelator_real_codes.h"
#include <gnuradio/block.h>
#include <fstream>
#include <string>

typedef struct
{
    /* DLL/PLL tracking configuration */
    double fs_in;
    unsigned int vector_length;
    bool dump;
    std::string dump_filename;
    float pll_bw_hz;
    float dll_bw_hz;
    float pll_bw_narrow_hz;
    float dll_bw_narrow_hz;
    float early_late_space_chips;
    float very_early_late_space_chips;
    float early_late_space_narrow_chips;
    float very_early_late_space_narrow_chips;
    int extend_correlation_symbols;
    bool track_pilot;
    char system;
    char signal[3];
} dllpllconf_t;

class dll_pll_veml_tracking;

typedef boost::shared_ptr<dll_pll_veml_tracking> dll_pll_veml_tracking_sptr;

dll_pll_veml_tracking_sptr dll_pll_veml_make_tracking(dllpllconf_t conf_);

/*!
 * \brief This class implements a code DLL + carrier PLL tracking block.
 */
class dll_pll_veml_tracking : public gr::block
{
public:
    ~dll_pll_veml_tracking();

    void set_channel(unsigned int channel);
    void set_gnss_synchro(Gnss_Synchro *p_gnss_synchro);
    void start_tracking();

    int general_work(int noutput_items, gr_vector_int &ninput_items,
        gr_vector_const_void_star &input_items, gr_vector_void_star &output_items);

    void forecast(int noutput_items, gr_vector_int &ninput_items_required);

private:
    friend dll_pll_veml_tracking_sptr dll_pll_veml_make_tracking(dllpllconf_t conf_);

    dll_pll_veml_tracking(dllpllconf_t conf_);

    bool cn0_and_tracking_lock_status(double coh_integration_time_s);
    bool acquire_secondary();
    void do_correlation_step(const gr_complex *input_samples);
    void run_dll_pll();
    void update_tracking_vars();
    void clear_tracking_vars();
    void save_correlation_results();
    void log_data(bool integrating);
    int save_matfile();

    // tracking configuration vars
    dllpllconf_t trk_parameters;
    bool d_veml;
    bool d_cloop;
    unsigned int d_channel;
    Gnss_Synchro *d_acquisition_gnss_synchro;

    //Signal parameters
    bool d_secondary;
    bool interchange_iq;
    double d_signal_carrier_freq;
    double d_code_period;
    double d_code_chip_rate;
    unsigned int d_secondary_code_length;
    unsigned int d_code_length_chips;
    unsigned int d_code_samples_per_chip;  // All signals have 1 sample per chip code except Gal. E1 which has 2 (CBOC disabled) or 12 (CBOC enabled)
    int d_symbols_per_bit;
    std::string systemName;
    std::string signal_type;
    std::string *d_secondary_code_string;

    //tracking state machine
    int d_state;
    bool d_synchonizing;
    //Integration period in samples
    int d_correlation_length_ms;
    int d_n_correlator_taps;

    float *d_tracking_code;
    float *d_data_code;
    float *d_local_code_shift_chips;
    float *d_prompt_data_shift;
    cpu_multicorrelator_real_codes multicorrelator_cpu;
    cpu_multicorrelator_real_codes correlator_data_cpu;  //for data channel
    /*  TODO: currently the multicorrelator does not support adding extra correlator
        with different local code, thus we need extra multicorrelator instance.
        Implement this functionality inside multicorrelator class
        as an enhancement to increase the performance
     */
    gr_complex *d_correlator_outs;
    gr_complex *d_Very_Early;
    gr_complex *d_Early;
    gr_complex *d_Prompt;
    gr_complex *d_Late;
    gr_complex *d_Very_Late;

    bool d_enable_extended_integration;
    int d_extend_correlation_symbols_count;
    int d_current_symbol;

    gr_complex d_VE_accu;
    gr_complex d_E_accu;
    gr_complex d_P_accu;
    gr_complex d_L_accu;
    gr_complex d_VL_accu;
    gr_complex d_last_prompt;

    gr_complex *d_Prompt_Data;

    double d_code_phase_step_chips;
    double d_carrier_phase_step_rad;
    // remaining code phase and carrier phase between tracking loops
    double d_rem_code_phase_samples;
    double d_rem_carr_phase_rad;

    // PLL and DLL filter library
    Tracking_2nd_DLL_filter d_code_loop_filter;
    Tracking_2nd_PLL_filter d_carrier_loop_filter;

    // acquisition
    double d_acq_code_phase_samples;
    double d_acq_carrier_doppler_hz;

    // tracking vars
    double d_carr_error_hz;
    double d_carr_error_filt_hz;
    double d_code_error_chips;
    double d_code_error_filt_chips;
    double d_K_blk_samples;
    double d_code_freq_chips;
    double d_carrier_doppler_hz;
    double d_acc_carrier_phase_rad;
    double d_rem_code_phase_chips;
    double d_code_phase_samples;
    double T_chip_seconds;
    double T_prn_seconds;
    double T_prn_samples;
    double K_blk_samples;
    // PRN period in samples
    int d_current_prn_length_samples;
    // processing samples counters
    unsigned long int d_sample_counter;
    unsigned long int d_acq_sample_stamp;

    // CN0 estimation and lock detector
    int d_cn0_estimation_counter;
    int d_carrier_lock_fail_counter;
    double d_carrier_lock_test;
    double d_CN0_SNV_dB_Hz;
    double d_carrier_lock_threshold;
    std::deque<gr_complex> d_Prompt_buffer_deque;
    gr_complex *d_Prompt_buffer;

    // file dump
    std::ofstream d_dump_file;
};

#endif  // GNSS_SDR_DLL_PLL_VEML_TRACKING_H
