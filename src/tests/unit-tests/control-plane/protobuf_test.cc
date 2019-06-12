/*!
 * \file protobuf_test.cc
 * \brief This file implements tests for Serdes_Gnss_Synchro
 * \author Carles Fernandez-Prades, 2019. cfernandez(at)cttc.es
 *
 * -------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019  (see AUTHORS file for a list of contributors)
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
 * along with GNSS-SDR. If not, see <https://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------
 */


#include "serdes_gnss_synchro.h"

TEST(Protobuf, Works)
{
    uint32_t prn_true = 17;
    uint32_t prn_true2 = 23;
    std::string sys = "G";
    std::string sig = "1C";

    Gnss_Synchro gs = Gnss_Synchro();
    gs.System = *sys.c_str();
    std::memcpy(static_cast<void*>(gs.Signal), sig.c_str(), 3);
    gs.PRN = prn_true;

    gs.Channel_ID = 3;

    gs.Acq_delay_samples = 431;
    gs.Acq_doppler_hz = 1234;
    gs.Acq_samplestamp_samples = 10000;
    gs.Acq_doppler_step = 125;
    gs.Flag_valid_acquisition = true;

    gs.fs = 10000000;
    gs.Prompt_I = 10000.0;
    gs.Prompt_Q = 0.01;
    gs.CN0_dB_hz = 39;
    gs.Carrier_Doppler_hz = 321;
    gs.Tracking_sample_counter = 11000;
    gs.Flag_valid_symbol_output = false;
    gs.correlation_length_ms = 1;

    gs.Flag_valid_word = false;
    gs.TOW_at_current_symbol_ms = 12345;

    gs.Pseudorange_m = 22000002.1;
    gs.RX_time = 4321;
    gs.Flag_valid_pseudorange = false;
    gs.interp_TOW_ms = 20;

    gs.Pseudorange_m = 22000002.1;
    gs.Carrier_phase_rads = 45.4;
    gs.Carrier_Doppler_hz = 321;
    gs.CN0_dB_hz = 39;

    Serdes_Gnss_Synchro serdes = Serdes_Gnss_Synchro();

    // Create a vector of Gnss_Synchro objects
    std::vector<Gnss_Synchro> vgs;
    vgs.push_back(gs);
    gs.PRN = prn_true2;
    vgs.push_back(gs);

    // Serialize data
    std::string serialized_data = serdes.createProtobuffer(vgs);

    // Recover data from serialization
    gnss_sdr::Observables obs;
    obs.ParseFromString(serialized_data);

    // Check that recovered data is ok

    // We can access like this:
    std::vector<Gnss_Synchro> vgs_read = serdes.readProtobuffer(obs);
    Gnss_Synchro gs_read = vgs_read[0];
    uint32_t prn_read = gs_read.PRN;
    uint32_t prn_read2 = vgs_read[1].PRN;

    // or without the need of gnss_synchro:
    int obs_size = obs.observable_size();
    uint32_t prn_read3 = obs.observable(0).prn();

    EXPECT_EQ(prn_true, prn_read);
    EXPECT_EQ(prn_true2, prn_read2);
    EXPECT_EQ(prn_true, prn_read3);
    EXPECT_EQ(2, obs_size);
}