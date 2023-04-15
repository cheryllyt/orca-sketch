from baseline_plot import BaselinePlot

ROW_NUMBERS_SMALL = [1,2,4]
ROW_NUMBERS = [1,2,3,4,6,10]

CS_ROW_NUMBERS = [1,3,5,9,15,31]

THRESHOLDS_SMALL = [0.0001, 0.000178, 0.000316, 0.000562, 0.001, 0.00178, 0.00316, 0.00562] # remove last 0.01 due to nan
THRESHOLDS = [0.0001, 0.000178, 0.000316, 0.000562, 0.001, 0.00178, 0.00316, 0.00562, 0.01]

baseline_plot = BaselinePlot()

# CMS #

# plot speed and error
baseline_plot.draw_subplot(True, False, 0.8, 'cms', ROW_NUMBERS, [], -1)
baseline_plot.draw_subplot(True, False, 1.5, 'cms', ROW_NUMBERS, [], -1)

# plot heavy hitter
baseline_plot.draw_subplot(False, True, 0.8, 'cms', ROW_NUMBERS_SMALL, THRESHOLDS_SMALL, 1024)
baseline_plot.draw_subplot(False, True, 1.5, 'cms', ROW_NUMBERS_SMALL, THRESHOLDS, 1024)

# CUS #

# plot speed and error
baseline_plot.draw_subplot(True, False, 0.8, 'cus', ROW_NUMBERS, [], -1)
baseline_plot.draw_subplot(True, False, 1.5, 'cus', ROW_NUMBERS, [], -1)

# plot heavy hitter
baseline_plot.draw_subplot(False, True, 0.8, 'cus', ROW_NUMBERS_SMALL, THRESHOLDS_SMALL, 1024)
baseline_plot.draw_subplot(False, True, 1.5, 'cus', ROW_NUMBERS_SMALL, THRESHOLDS, 1024)

# CS #

# plot speed and error
baseline_plot.draw_subplot(True, False, 0.8, 'cs', CS_ROW_NUMBERS, [], -1)
baseline_plot.draw_subplot(True, False, 1.5, 'cs', CS_ROW_NUMBERS, [], -1)

# plot heavy hitter
baseline_plot.draw_subplot(False, True, 0.8, 'cms', [1], THRESHOLDS_SMALL, 1024)
baseline_plot.draw_subplot(False, True, 1.5, 'cms', [1], THRESHOLDS, 1024)

baseline_plot.show_plots()
