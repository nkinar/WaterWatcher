# Constants

FIG_SIZE = (14, 6)
OUT_DIR = './out/'
FIGURE_DIR = OUT_DIR
DPI = 300
OUT_EXT = '.png'
FONT_SIZ = 12

FIG_SIZE_TIME_PLOTS = (12, 9)

FIELD_SAMPLE_INTERVAL = 30              # minutes
DT_SEC = FIELD_SAMPLE_INTERVAL * 60     # seconds
FS_SEC = 1 / DT_SEC                     # frequency

##############################################
# Turbidity min and max to constrain
MIN_TURB = 0.0      # NTU
MAX_TURB = 4000     # NTU

MIN_TDS = 0.0       # ppm
MAX_TDS = 3000      # ppm

DV_RES = 0.8e-3  # 0.8 mV
##############################################

# time and date string
PYTHON_TIME_DATE_STR = '%d/%m/%Y %H:%M:%S'

# names of objects to be stored to pickle file
TURBIDITY_MODEL_OBJ = 'turbidity_model_p'
TDS_MODEL_COEFF = 'model_tdsc'
TDS_MODEL_INDICES = 'lst_index_tdsc'
TSS_TURB_M = 'tss_turbidity_m'

MIN_TURBIDITY_ACTUAL = 'min_turbidity'
MAX_TURBIDITY_ACTUAL = 'max_turbidity'
MIN_TDS_ACTUAL = 'min_tds'
MAX_TDS_ACTUAL = 'max_tds'

# Data file names
fn_dut = '../data/calibration-data/NONAME_parsed.csv'
fn_exosonde = '../data/calibration-data/KorEXO_Measurement_File_Export_021520_211205_parsed-same-cols.csv'
fn_field_data = '../data/deployment/field-test.csv'
fn_precip_data = '../data/deployment/weather-station-2020-june-july.csv'
fn_turbidity_tss = '../data/ancillary/turbidity-and-tss-and-tds.csv'
fn_comparison_wq = '../data/ancillary/water-sample-comparison.csv'

# Output file names
FN_CALIB_OUT = OUT_DIR + 'calib.txt'
FN_CALIB_COEFF = OUT_DIR + 'calibration-coeff.pickle'
FN_TSS_TURB_COEFF = OUT_DIR + 'tss-turbidity-coeff.pickle'

# Figure names
FIG_CALIB = FIGURE_DIR + 'calibration' + OUT_EXT
FIG_FIELD = FIGURE_DIR + 'field-data' + OUT_EXT
SEMIVAR_PLOT = FIGURE_DIR + 'semivar' + OUT_EXT




