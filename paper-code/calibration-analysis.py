from numpy import genfromtxt, poly1d, polyfit
import matplotlib.pyplot as plt
from compute_rmsd_mb_arrays import *
from dir_tools import make_dir
from constants import *
from polyfit2d import polyfit2d, polyfit2d_evaluate_vec
from compute_rmsd_mb_arrays import compute_rmsd, compute_mb
from polynomial_inc_dec import constrained_polyfit, epoly
from scipy.interpolate import UnivariateSpline
import pickle
from plotTools import make_axes_tight, set_figure_text_size, use_ggplot_style, turn_ticklabels_off_xaxis
from WaterWatcherProcess import WaterWatcherProcess
import datetime
import dateparser
from scipy import signal
from normalize import normalize_matrix
from semivariogram import semivariogram_xz
from constant_labels import create_label, temp_label
from addtick import addtick_x
from number_helper import get_closest_date


def load_precip_data():
    precip_data = genfromtxt(fn_precip_data, delimiter=',', dtype=str)
    precip_time = precip_data[1:, 4]
    precip_mm = precip_data[1:, 12]
    time_precip = []
    n = length(precip_time)
    for k in range(n):
        out_time = dateparser.parse(precip_time[k])
        time_precip.append(out_time)
    return time_precip, precip_mm.astype(float)


def load_field_data():
    field_data = genfromtxt(fn_field_data, delimiter=',', dtype=str)
    dut_ntu_voltage_col = 0
    dut_temperature_col = 2
    dut_tds_voltage_col = 4
    dut_end_time_col = 12
    dut_battery = 14
    dut_charging = 9
    dut_current_col = 15
    dut_ntu_voltage = field_data[1:, dut_ntu_voltage_col]
    dut_tds_voltage = field_data[1:, dut_tds_voltage_col]
    dut_temperature = field_data[1:, dut_temperature_col ]
    dut_time = field_data[1:, dut_end_time_col]
    dut_bvoltage = field_data[1:, dut_battery]
    dut_chargingflag = field_data[1:, dut_charging]
    dut_current = field_data[1:, dut_current_col]
    n = length(dut_time)
    dut_time_p = []
    for k in range(n):
        tout = datetime.datetime.strptime(dut_time[k], PYTHON_TIME_DATE_STR)
        dut_time_p.append(tout)
    return dut_time_p, dut_temperature.astype(float), dut_ntu_voltage.astype(float), dut_tds_voltage.astype(float),  \
           dut_bvoltage.astype(float), dut_chargingflag.astype(float), dut_current.astype(float)


def load_calib_data(get_date_string=False):

    data_dut = genfromtxt(fn_dut, delimiter=',', dtype=str)
    data_exo = genfromtxt(fn_exosonde, delimiter=',', dtype=str)

    exo_date = data_exo[1:, 0]
    exo_time = data_exo[1:, 1]
    exo_tds_col = 11
    exo_turbidity_col = 12
    exo_temperature_col = 17

    exo_tds = data_exo[1:, exo_tds_col]
    exo_turbidity = data_exo[1:, exo_turbidity_col]
    exo_temperature = data_exo[1:, exo_temperature_col]

    dut_ntu_voltage_col = 0
    dut_tds_voltage_col = 4
    dut_end_time_col = 12
    dut_ntu_voltage = data_dut[1:, dut_ntu_voltage_col]
    dut_tds_voltage = data_dut[1:, dut_tds_voltage_col]
    dut_time = data_dut[1:, dut_end_time_col]

    if get_date_string:
        return dut_time, exo_tds.astype(float), \
               exo_turbidity.astype(float), \
               exo_temperature.astype(float), \
               dut_ntu_voltage.astype(float), \
               dut_tds_voltage.astype(float)
    # return without time string
    return exo_tds.astype(float), \
           exo_turbidity.astype(float), \
           exo_temperature.astype(float), \
           dut_ntu_voltage.astype(float), \
           dut_tds_voltage.astype(float)


def compute_mr_resolution_x(v, dv, f):
    """
    Function to compute the resolution
    :param v:           as the voltage
    :param dv:          as the change in voltage
    :param f:           function to check
    :param opt:         any additional options passed to the function
    :return:            the resolution of the sensor
    """
    n = length(v)
    ssum = 0
    for k in range(1, n):
        ssum += f(v[k]) - f(v[k] + dv)
    out = np.abs(ssum / float(n))
    return out


def compute_mr_resolution_xy(v, t, dv, f):
    """
    Function to compute the resolution
    :param v:           as the voltage
    :param t:           as the temperature
    :param dv:          as the change in voltage
    :param f:           function to check
    :param opt:         any additional options passed to the function
    :return:            the resolution of the sensor
    """
    n = length(v)
    if n != length(t):
        raise Exception('length(x) != length(y)')
    ssum = 0
    for k in range(1, n):
        ssum += f(v[k], t[k]) - f(v[k] + dv, t[k])
    out = np.abs(ssum / float(n))
    return out


def run_analysis_calibrate(show=False):
    # make the output directory
    make_dir(FIGURE_DIR)

    # Load the data
    exo_tds, exo_turbidity, exo_temperature, dut_ntu_voltage, dut_tds_voltage = load_calib_data()

    # Remove the data at the beginning of the experiment since equilibrium has not been reached
    ncut = 35
    exo_tds0 = exo_tds[ncut:]
    exo_turbidity0 = exo_turbidity[ncut:]
    exo_temperature0 = exo_temperature[ncut:]
    dut_ntu_voltage0 = dut_ntu_voltage[ncut:]
    dut_tds_voltage0 = dut_tds_voltage[ncut:]

    ##################################################
    # TDS
    ##################################################
    fn_out = open(FN_CALIB_OUT, 'w')

    # Initial polynomial curve-fiting for TDS voltage curve
    tds_order_initial = 7
    x_tdsv = np.arange(0, length(dut_tds_voltage0))
    fit_tdsv = polyfit(x_tdsv, dut_tds_voltage0, tds_order_initial)
    model_tds = poly1d(fit_tdsv)
    y_tdsv = model_tds(x_tdsv)

    # Fit a 2D polynomial for g(voltage, temperature) = TDS
    xp = y_tdsv                 # TDS voltage
    yp = exo_temperature0       # temperature
    zp = exo_tds0               # known TDS
    zpp = np.tile(exo_tds0, (length(xp), 1))
    kx = 7  # order in the x for the polynomial
    ky = 7  # order in the y for the polynomial
    model_tdsc, lst_index_tdsc = polyfit2d(xp, yp, zpp, kx=kx, ky=ky, order=None)
    zp_model = polyfit2d_evaluate_vec(xp, yp, model_tdsc, lst_index_tdsc)
    rmsd_tds_model = compute_rmsd(zp, zp_model)
    mb_tds_model = compute_mb(zp, zp_model)
    fn_out.write('RMSD TDS = {0:.4} ppm\n'.format(rmsd_tds_model))
    fn_out.write('MB TDS = {0:.4} ppm\n'.format(mb_tds_model))

    ##################################################
    # Turbidity
    ##################################################
    poly_order_fit_turb = 5
    x_turbv = np.arange(0, length(dut_ntu_voltage0))
    fit_turbv = polyfit(x_turbv, dut_ntu_voltage0, poly_order_fit_turb )
    model_turbv = poly1d(fit_turbv)
    turbv = model_turbv(x_turbv)

    order = 5
    slack = 1.0e-3
    xelem = np.linspace(0, length(turbv), length(turbv))
    c_updated = constrained_polyfit(xelem, turbv, order, 'dec', slack, ndiscret=None)
    turb_vmodel = epoly(xelem, c_updated)

    turb_vmodel2 = turb_vmodel[::-1]
    exo_turbidity2 = exo_turbidity0

    # Cubic spline interpolation
    turbidity_model_p = UnivariateSpline(turb_vmodel2, exo_turbidity0)
    turbidity_model = turbidity_model_p(turb_vmodel2)
    rmsd_turb_model = compute_rmsd(exo_turbidity0, turbidity_model)
    mb_turb_model = compute_mb(exo_turbidity0, turbidity_model)
    fn_out.writelines('RMSD Turbidity = {0:.4} NTU\n'.format(rmsd_turb_model))
    fn_out.writelines('MB Turbidity = {0:.4} NTU\n'.format(mb_turb_model))

    # Spline model relating turbidity to voltage
    # f(V) = turbidity
    # turbidity_model_p

    # Model coefficients relating tds to voltage and water temperature
    # g(V, T) = TDS
    # model_tdsc, lst_index_tdsc
    dcoeff = {TURBIDITY_MODEL_OBJ: turbidity_model_p,
              TDS_MODEL_COEFF: model_tdsc,
              TDS_MODEL_INDICES: lst_index_tdsc
              }

    # Save the coefficients out to a pickle file
    with open(FN_CALIB_COEFF, 'wb') as fpick:
        pickle.dump(dcoeff, fpick)

    # compute the resolution
    wwp = WaterWatcherProcess()
    # print(wwp.turbidity(0.1))
    # print(wwp.tds(1.5, 15))

    # resolution for turbidity
    turbres = compute_mr_resolution_x(turb_vmodel2, DV_RES, wwp.turbidity)
    fn_out.writelines('Turbidity Resolution = {0:.4} NTU\n'.format(turbres))

    # resolution for tds
    tdsres = compute_mr_resolution_xy(xp, yp, DV_RES, wwp.tds)[0]
    fn_out.writelines('TDS Resolution = {0:.4} ppm\n'.format(tdsres))

    # close the file that writes out the results
    fn_out.close()

    # Turbidity Plot
    plot_turbidity = False
    if plot_turbidity:
        plt.figure()
        plt.plot(exo_turbidity0)
        plt.title('EXO Turbidity')
        plt.ylabel('NTU')
        plt.xlabel('Sample')
        block = False
        plt.show(block=block)

        plt.figure()
        plt.plot(x_turbv, dut_ntu_voltage0, label='voltage')
        plt.plot(x_turbv, turbv, label='model')
        plt.plot(xelem, turb_vmodel, label='model dec')
        plt.title('Turbidity voltage')
        plt.xlabel('Sample')
        plt.ylabel('Voltage')
        plt.legend()
        block = False
        plt.show(block=block)

        plt.figure()
        plt.plot(turb_vmodel, exo_turbidity0, label='voltage')
        plt.title('Turbidity vs Voltage')
        plt.xlabel('Voltage')
        plt.ylabel('Turbidity (NTU)')
        block = False
        plt.show(block=block)

        plt.figure()
        plt.plot(turb_vmodel2, exo_turbidity0, 'bo', label='voltage')
        plt.plot(turb_vmodel2, turbidity_model, label='model')
        plt.title('Turbidity vs Voltage (Ordered)')
        plt.xlabel('Voltage')
        plt.ylabel('Turbidity (NTU)')
        plt.legend()
        block = True
        plt.show(block=block)

    # TDS plots
    plot_tds = False
    if plot_tds:
        plt.figure()
        plt.plot(exo_tds0)
        plt.title('EXO TDS')
        block = False
        plt.show(block=block)

        plt.figure()
        plt.plot(y_tdsv, exo_tds0, label='data')
        plt.plot(y_tdsv, zp_model, label='model')
        plt.title('EXO TDS vs DUT Voltage')
        plt.legend()
        plt.xlabel('Voltage')
        plt.ylabel('TDS (ppm)')
        block = False
        plt.show(block=block)

        plt.figure()
        plt.plot(exo_temperature0)
        plt.title('EXO Temperature')
        block = False
        plt.show(block=block)

        plt.figure()
        plt.plot(dut_tds_voltage0)
        plt.plot(x_tdsv, y_tdsv)
        plt.title('TDS voltage')
        block = True
        plt.show(block=block)

    plot_export = True
    if plot_export:

        # create the output figure
        plt.figure(figsize=FIG_SIZE)
        ax1 = plt.subplot(121)

        # Turbidity
        plt.title('(a)')
        plt.plot(turb_vmodel2, exo_turbidity0, 'o', color='blue', label='data')
        plt.plot(turb_vmodel2, turbidity_model, label='model')
        plt.xlabel('Voltage (V)')
        plt.ylabel('Turbidity (NTU)')
        plt.xlim([0, 2.5])

        # TDS
        ax2 = plt.subplot(122)
        plt.title('(b)')

        plt.plot(y_tdsv, exo_tds0, 'o', color='blue', label='data')
        plt.plot(y_tdsv, zp_model, label='model')
        plt.xlabel('Voltage (V)')
        plt.ylabel('TDS (ppm)')
        plt.legend()
        plt.xlim([0, 2.5])

        plt.tight_layout()
        plt.savefig(FIG_CALIB, dpi=DPI)
        if show:
            plt.show(block=True)


def run_process_data(show=False):
    # create the object to deal with the data
    wwp = WaterWatcherProcess()
    mt = wwp.turbidity(4.5)  # offset removal coefficient
    dtemp = 7                # offset removal coefficient

    # load in the data collected in the field (only at the start and end points)
    ncut_start = 103
    ncut_end = 795
    dut_time_p, dut_temperature, dut_ntu_voltage, dut_tds_voltage, dut_battery_voltage, dut_chargingflag, dut_current, \
        = load_field_data()
    dut_time_p0 = dut_time_p[ncut_start:ncut_end+1]
    dut_temperature0 = dut_temperature[ncut_start:ncut_end+1]
    dut_ntu_voltage0 = dut_ntu_voltage[ncut_start:ncut_end+1]
    dut_tds_voltage0 = dut_tds_voltage[ncut_start:ncut_end+1]
    dut_battery_voltage0 = dut_battery_voltage[ncut_start:ncut_end+1]
    dut_chargingflag0 = dut_chargingflag[ncut_start:ncut_end+1]
    dut_current0mA = np.abs(dut_current[ncut_start:ncut_end+1]) * 1000

    min_bat = np.min(dut_battery_voltage0)
    max_bat = np.max(dut_battery_voltage0)
    av_current = np.mean(dut_current0mA)
    print('min bat = {:.4} V'.format(min_bat))
    print('max bat = {:.4} V'.format(max_bat))
    print('av_current = {:.4} mA'.format(av_current))

    # load in the comparison data and the m-slope of the line used to convert TSS into turbidity
    d_tss_turbidity = pickle.load(open(FN_TSS_TURB_COEFF, "rb"))
    m_tss_turbidity = d_tss_turbidity[TSS_TURB_M]
    comp_data = genfromtxt(fn_comparison_wq, delimiter=',', dtype=str)
    comp_data_date = [dateparser.parse(i) for i in comp_data[1:, 0]]    # date as col 0
    comp_data_tds_ppm = np.asarray(comp_data[1:, 1], dtype=float)       # TDS (ppm) as col 1
    comp_data_tss = np.asarray(comp_data[1:, 2], dtype=float)           # TSS (mg/L) as col 2
    # convert TSS into turbidity (NTU)
    comp_data_turbidity_ntu = m_tss_turbidity * comp_data_tss

    # min and max values
    min_turbidity = d_tss_turbidity[MIN_TURBIDITY_ACTUAL]
    max_turbidity = d_tss_turbidity[MAX_TURBIDITY_ACTUAL]
    min_tds = d_tss_turbidity[MIN_TDS_ACTUAL]
    max_tds = d_tss_turbidity[MAX_TURBIDITY_ACTUAL]

    #############################################################################
    # Turbidity and TDS data
    #############################################################################
    turbidity_series = wwp.turbidity(dut_ntu_voltage0) - mt
    tds_series = wwp.tds(dut_tds_voltage0, dut_temperature0-dtemp)
    indx = tds_series > 480     # remove outlier
    tds_series[indx] = np.NaN   # set outlier to NaN
    dut_time_p0_tds = dut_time_p0
    time_precip, precip_mm = load_precip_data()

    #############################################################################
    # Spectrogram
    #############################################################################
    tstart = dut_time_p0[0]
    tend = dut_time_p0[-1]
    tdiff = tend-tstart
    tseconds = tdiff.total_seconds()
    f_turbidity, t_turbidity, Sxx_turbidity = signal.spectrogram(turbidity_series)
    f_tds, t_tds, Sxx_tds = signal.spectrogram(tds_series)

    # normalize to interval [0, 1]
    Sxx_turbidity_n = normalize_matrix(Sxx_turbidity)
    Sxx_tds_n = normalize_matrix(Sxx_tds)

    #############################################################################
    # Semivariogram for Turbidity
    #############################################################################
    dtm = 30            # minutes
    dt = dtm*60         # 30 minutes in seconds
    nntmin = 9400          # minutes for semivariograms

    # Turbidity
    nseries_turb = length(turbidity_series)
    tseries_turb = np.linspace(0, nseries_turb*dtm-dtm, nseries_turb)
    hturbidity = np.linspace(0, nntmin, 100)
    hturbidity0, gamma_turbidity = semivariogram_xz(tseries_turb, turbidity_series, h=hturbidity)

    # TDS
    nseries_tds = length(tds_series)
    tseries_tds = np.linspace(0, nseries_tds * dtm - dtm, nseries_tds)
    htds = np.linspace(0, nntmin, 100)
    htds0, gamma_tds = semivariogram_xz(tseries_tds, tds_series, h=htds)

    # Temperature
    nseries_temp = length(dut_temperature0)
    tseries_temp = np.linspace(0, nseries_temp * dtm - dtm, nseries_temp)
    htemp = np.linspace(0, nntmin, 100)
    htemp0, gamma_temp = semivariogram_xz(tseries_temp, dut_temperature0, h=htemp)

    #############################################################################
    # Plot of turbidity, TDS and temperature along with spectrograms
    #############################################################################
    bbox = dict(facecolor='white', alpha=1)

    # Turbidity plot
    fig1 = plt.figure(figsize=FIG_SIZE_TIME_PLOTS)
    ax1 = plt.subplot(611)

    ax1.hlines(y=min_turbidity, xmin=dut_time_p0[0], xmax=dut_time_p0[-1], color='gray', label='Min/Max Measured')
    ax1.hlines(y=max_turbidity, xmin=dut_time_p0[0], xmax=dut_time_p0[-1], color='gray')

    plt.plot(dut_time_p0, turbidity_series, color='cornflowerblue', label='Automated')
    plt.plot(comp_data_date, comp_data_turbidity_ntu, marker='o', linestyle='', label='Manual Measured', color='darkviolet')
    plt.ylim([-200, 3000])
    plt.ylabel('Turbidity (NTU)')
    turn_ticklabels_off_xaxis()
    ax1.set_title('(a)', loc='right', y=0.75, bbox=bbox)
    ax1.legend(loc='upper left')

    # Spectrogram of turbidity plot
    ax2 = plt.subplot(612)
    pm1 = plt.pcolormesh([0, tseconds], f_turbidity, Sxx_turbidity_n, shading='gouraud')
    plt.ylabel('FF')
    plt.ylim([0, 0.04])
    plt.autoscale(enable=True, axis='x', tight=True)
    turn_ticklabels_off_xaxis()
    ax2.set_title('(b)', loc='right', y=0.75, bbox=bbox)

    # TDS plot
    ax3 = plt.subplot(613)
    rm = ~np.isnan(tds_series)
    dut_time_p0_tds_p = np.asarray(dut_time_p0_tds)[rm]
    tds_series_p = tds_series[rm]
    # min and max lines
    ax3.hlines(y=min_tds, xmin=dut_time_p0_tds_p[0], xmax=dut_time_p0_tds_p[-1], color='gray')
    ax3.hlines(y=max_tds, xmin=dut_time_p0_tds_p[0], xmax=dut_time_p0_tds_p[-1], color='gray')

    plt.plot(dut_time_p0_tds_p, tds_series_p, color='cornflowerblue', label='Automated')
    plt.plot(comp_data_date, comp_data_tds_ppm, marker='o', linestyle='', label='Manual', color='darkviolet')
    turn_ticklabels_off_xaxis()
    plt.ylabel('TDS (ppm)')
    plt.ylim([0, 700])
    ax3.set_title('(c)', loc='right', y=0.75, bbox=bbox)

    # Spectrogram of TDS plot
    ax4 = plt.subplot(614)
    pm2 = plt.pcolormesh([0, tseconds], f_tds, Sxx_tds_n, shading='gouraud')
    plt.ylabel('FF')
    plt.ylim([0, 0.04])
    plt.autoscale(enable=True, axis='x', tight=True)
    turn_ticklabels_off_xaxis()
    ax4.set_title('(d)', loc='right', y=0.75, bbox=bbox)

    # Precipitation and Water Temperature
    ax5 = plt.subplot(615)
    ax5temp = ax5.twinx()
    ax5.set_ylabel('Precip (mm)')
    ax5.bar(time_precip, precip_mm, color='cornflowerblue', label='Precipitation', align='edge', width=0.1)
    turn_ticklabels_off_xaxis()
    ax5.set_xlim([dut_time_p0[0], dut_time_p0[-1]])
    ax5.set_title('(e)', loc='right', x=1.1, y=0.5, bbox=bbox)
    ax5temp.plot(dut_time_p0, dut_temperature0, color='skyblue', label='Water Temperature')
    ax5temp.set_ylabel(temp_label(small=True))

    # Battery voltage and current
    ax6 = plt.subplot(616)
    ax6ma = ax6.twinx()
    ax6.plot(dut_time_p0, dut_battery_voltage0, color='orangered', label='Voltage')
    ax6.set_ylabel('Volts')
    ax6ma.plot(dut_time_p0, dut_current0mA, color='blue', label='Current')
    ax6ma.set_ylabel('Current (mA)')
    bar_height = 10
    ax6.bar(dut_time_p0, bar_height * dut_chargingflag0, color='seagreen', label='Charge Event', align='edge', width=0.1)
    ax6.set_xlim([dut_time_p0[0], dut_time_p0[-1]])
    ax6.set_title('(f)', loc='right', x=1.1, y=0.5, bbox=bbox)

    # Spectrogram colorbar at top of plots
    cax = fig1.add_axes([0.1, .95, 0.3, 0.03])  # Dependent on the layout and may have to be adjusted
    cbar = fig1.colorbar(pm1, orientation='horizontal', cax=cax)
    cbar.set_label('Normalized PSD')

    # legend
    ax6.legend(bbox_to_anchor=(1, -0.2))
    ax6ma.legend(bbox_to_anchor=(0.82, -0.2))
    ax5.legend(loc='upper right')
    ax5temp.legend(loc='upper left')

    # save out the figure
    plt.savefig(FIG_FIELD, dpi=DPI)
    plt.show(block=False)

    # TURBIDITY COMPARISONS
    known_time_turbidity = comp_data_date[0]
    known_turbidity = comp_data_turbidity_ntu[0]
    close_time_turbidity, close_time_turbidity_indx = get_closest_date(known_time_turbidity, dut_time_p0)
    ww_turbidity = turbidity_series[close_time_turbidity_indx]
    mb_turbidity = compute_mb(known_turbidity, ww_turbidity)
    mb_turbidity_all = compute_mb(known_turbidity * np.ones(length(turbidity_series)), turbidity_series)
    rmsd_turbidity_all = compute_rmsd(known_turbidity * np.ones(length(turbidity_series)), turbidity_series)
    print('-----Turbidity-----')
    print('mb_turbidity sample = {:.2f} NTU'.format(mb_turbidity))
    print('mb_turbidity all = {:.2f} NTU'.format(mb_turbidity_all))
    print('rmsd_turbidity all = {:.2f} NTU'.format(rmsd_turbidity_all))

    # TDS COMPARISONS
    known_time_tds = comp_data_date[0]
    known_tds = comp_data_tds_ppm[0]
    close_time_tds, close_time_tds_indx = get_closest_date(known_time_tds, dut_time_p0_tds_p)
    ww_tds = tds_series[close_time_tds_indx]
    mb_tds = compute_mb(known_tds, ww_turbidity)
    mb_tds_all = compute_mb(known_tds * np.ones(length(tds_series)), tds_series)
    rmsd_tds_all = compute_rmsd(known_tds * np.ones(length(tds_series)), tds_series)
    print('-----TDS-----')
    print('mb_tds sample = {:.2f} ppm'.format(mb_tds))
    print('mb_tds all = {:.2f} ppm'.format(mb_tds_all))
    print('rmsd_tds all = {:.2f} ppm'.format(rmsd_tds_all))

    # #############################################################################
    # # Semivariogram
    # #############################################################################
    fig2 = plt.figure(figsize=FIG_SIZE)
    ax1 = plt.subplot(311)
    plt.plot(hturbidity0, gamma_turbidity, '-', color='cornflowerblue', markersize=5, linewidth=2)
    plt.ylim([0, 110000])
    plt.title('(a)')
    plt.ylabel(create_label('$\gamma$', 'NTU^2'))
    xturb_range = 9100  # minutes
    plt.axvline(x=xturb_range, linestyle='--')
    addtick_x(xturb_range, clear=True)
    plt.autoscale(enable=True, axis='x', tight=True)

    ax2 = plt.subplot(312)
    plt.plot(htds0, gamma_tds, '-', color='cornflowerblue', markersize=5, linewidth=2)
    plt.title('(b)')
    plt.ylabel(create_label('$\gamma$', 'ppm^2'))
    xtds_range = 8000  # minutes
    plt.axvline(x=xtds_range, linestyle='--')
    addtick_x(xtds_range, clear=True)
    plt.autoscale(enable=True, axis='x', tight=True)

    ax2 = plt.subplot(313)
    plt.plot(htemp0, gamma_temp, '-', color='cornflowerblue', markersize=5, linewidth=2)
    plt.title('(c)')
    plt.xlabel('Time (minutes)')
    plt.ylim([0, 15])
    xtemp_range = 9200  # minutes
    plt.axvline(x=xtemp_range, linestyle='--')
    addtick_x(xtemp_range, clear=False)
    plt.ylabel(r'$\gamma\hspace{0.3}(^{\circ}\mathrm{C}^2)$')
    plt.autoscale(enable=True, axis='x', tight=True)

    plt.tight_layout()
    plt.savefig(SEMIVAR_PLOT, dpi=DPI)
    if show:
        plt.show(block=True)


def obtain_tss_turbidity_relationship_and_tds(block):
    data = genfromtxt(fn_turbidity_tss, delimiter=',', dtype=str)
    turbidity_col = 2
    tss_col = 3
    tds_col = 4
    turbidity = np.asarray(data[1:, turbidity_col], dtype=float)
    tss = np.asarray(data[1:, tss_col], dtype=float)
    tds = np.asarray(data[1:, tds_col], dtype=float)

    min_turbidity = np.min(turbidity)
    max_turbidity = np.max(turbidity)
    min_tds = np.min(tds)
    max_tds = np.max(tds)

    x = tss
    x = x[:, np.newaxis]
    y = turbidity
    # force the fit through zero since 0 NTU must coincide with 0 mg/L
    m, _, _, _ = np.linalg.lstsq(x, y, rcond=None)
    m = m.item()
    xlin = np.linspace(0, np.max(tss))
    t_y = m*xlin

    dcoeff = {
        TSS_TURB_M: m,
        MIN_TURBIDITY_ACTUAL: min_turbidity,
        MAX_TURBIDITY_ACTUAL: max_turbidity,
        MIN_TDS_ACTUAL: min_tds,
        MAX_TDS_ACTUAL: max_tds
    }
    with open(FN_TSS_TURB_COEFF, 'wb') as fpick:
        pickle.dump(dcoeff, fpick)

    plt.figure()
    plt.plot(tss, turbidity, marker='o', linestyle='')
    plt.plot(xlin, t_y)
    plt.xlabel('TSS (mg/L)')
    plt.ylabel('Turbidity (NTU)')
    plt.show(block=block)
# DONE



#############################################################################


def main():
    make_axes_tight()
    set_figure_text_size(FONT_SIZ)
    use_ggplot_style()

    show = False
    obtain_tss_turbidity_relationship_and_tds(show)
    run_analysis_calibrate(show)
    run_process_data(show)


if __name__ == '__main__':
    main()

