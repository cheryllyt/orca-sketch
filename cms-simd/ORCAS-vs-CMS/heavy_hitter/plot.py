import os
import matplotlib.pyplot as plt
import statistics

ORCAS = 'ORCAS'
CMS = 'CMS'

SKETCHES = [ORCAS, CMS]
THRESHOLDS = [0.0001, 0.000178, 0.000316, 0.000562, 0.001, 0.00178, 0.00316, 0.00562, 0.01]

def get_all_values(hh_data, fixed_orcas, fixed_cms):

    hh_dict = {}
    
    for sketch in SKETCHES:

        if sketch == ORCAS:
            for array_counter in fixed_orcas:
                key = ORCAS + '-' + str(array_counter)

                hh_dict[key] = {}

                sketch_hh_data = hh_data[sketch]
                for line in sketch_hh_data:
                    split_line = line.split('\t')
                    sketch_size = int(split_line[3])
                    n_arrays = int(split_line[5])
                    n_array_counters = int(split_line[7])
                    rel_error = []
                    j = 11
                    for threshold in THRESHOLDS:
                        rel_error.append(float(split_line[j]))
                        j += 4

                    if n_array_counters == array_counter:
                        if sketch_size not in hh_dict[key]:
                            hh_dict[key][sketch_size] = []
                        hh_dict[key][sketch_size].append(rel_error)

        elif sketch == CMS:
            for row_number in fixed_cms:
                key = CMS + '-' + str(row_number)

                hh_dict[key] = {}

                sketch_hh_data = hh_data[sketch]
                for line in sketch_hh_data:
                    split_line = line.split('\t')
                    width = int(split_line[3])
                    n_rows = int(split_line[5])
                    rel_error = []
                    j = 9
                    for threshold in THRESHOLDS:
                        rel_error.append(float(split_line[j]))
                        j += 4

                    if n_rows == row_number:
                        if width not in hh_dict[key]:
                            hh_dict[key][width] = []
                        hh_dict[key][width].append(rel_error)

    return hh_dict

def get_median(hh_dict_all):

    hh_dict = {}

    for key in hh_dict_all:
        hh_dict[key] = []

        for size in hh_dict_all[key]:
            meds = []
            n_seeds = len(hh_dict_all[key][size])
            for i in range(len(THRESHOLDS)):
                to_compare = []
                for j in range(n_seeds):
                    to_compare.append(hh_dict_all[key][size][j][i])
                med = statistics.median(to_compare)
                meds.append(med)
            hh_dict[key].append((size, meds))

    return hh_dict

def plot_speed_and_error(alpha:float, fixed_orcas:int, fixed_cms:int, fixed_sketch_size=None): # ORCAS array counters, CMS row numbers

    # alpha must be 3 char (1 digit before '.' and 1 digit after)
    alpha_str = str(alpha)[0] + '-' + str(alpha)[-1]
    alpha_folder_name = '../alpha_' + alpha_str + '/'

    hh_data = {}

    for sketch in SKETCHES:
        hh_data[sketch] = []

    # e.g. alpha_1-0/seed_1_test_orcas_error_on_arrival.txt
    for fn in os.listdir(alpha_folder_name):
        indiv_data = open(alpha_folder_name + fn).readlines()

        if ORCAS.lower() in fn:
            indiv_data = indiv_data[1:] # remove zipf generation line
            if 'final_error' in fn:
                hh_data[ORCAS] = hh_data[ORCAS] + indiv_data

        elif CMS.lower() in fn:
            if 'final_error' in fn:
                hh_data[CMS] = hh_data[CMS] + indiv_data
    
    hh_dict_all = get_all_values(hh_data, fixed_orcas, fixed_cms)
    hh_dict = get_median(hh_dict_all)

    fig, (hh_ax) = plt.subplots(1, 1)

    for key in hh_dict:
        
        if ORCAS in key:
            key_label = key.split('-')[0] + ': ' + key.split('-')[1]

            hh_x = [threshold for threshold in THRESHOLDS]
            hh_y = []
            for (sketch_size, rel_error) in hh_dict[key]:
                if (sketch_size) == fixed_sketch_size:
                    hh_y = rel_error
                    break
            hh_ax.plot(hh_x, hh_y, label=str(key_label))

        elif CMS in key:
            row_number = int(key.split('-')[1])
            key_label = key.split('-')[0] + ': ' + key.split('-')[1]

            hh_x = [threshold for threshold in THRESHOLDS]
            hh_y = []
            for (width, rel_error) in hh_dict[key]:
                if (row_number*width) == fixed_sketch_size:
                    hh_y = rel_error
                    break
            hh_ax.plot(hh_x, hh_y, label=str(key_label))

    N = hh_data[ORCAS][0].split('\t')[1]
    fig_title = 'N = ' + N + ' | alpha = ' + str(alpha) + ' | ORCAS config: array counter | CMS config: row number'
    if fixed_sketch_size is not None:
        fig_title = fig_title + ' | fixed sketch size = ' + str(fixed_sketch_size)
    fig.suptitle(fig_title, fontsize=14)

    hh_ax.set_xlabel('Threshold', fontsize=18)
    hh_ax.set_ylabel('ARE', fontsize=18)
    hh_ax.tick_params(axis='both', which='both', labelsize=15)
    hh_ax.set_xscale('log',base=10)
    hh_ax.set_yscale('log',base=10)
    hh_ax.legend(loc="upper right", prop={"size":11})

def plot_all_alpha():
    
    # plot_speed_and_error(1.0, fixed_orcas=[3,4], fixed_cms=[1,2,4], fixed_sketch_size=1024)
    # plot_speed_and_error(1.0, fixed_orcas=[3,4], fixed_cms=[1,2,4], fixed_sketch_size=65536)
    # plot_speed_and_error(1.0, fixed_orcas=[3,4], fixed_cms=[1,2,4], fixed_sketch_size=4194304)
    # plot_speed_and_error(1.0, fixed_orcas=[3,4], fixed_cms=[1,2,4], fixed_sketch_size=8388608)

    plot_speed_and_error(1.5, fixed_orcas=[3,4], fixed_cms=[1,2,4], fixed_sketch_size=65536)
    plot_speed_and_error(1.5, fixed_orcas=[3,4], fixed_cms=[1,2,4], fixed_sketch_size=4194304)
    plot_speed_and_error(0.8, fixed_orcas=[3,4], fixed_cms=[1,2,4], fixed_sketch_size=65536)
    plot_speed_and_error(0.8, fixed_orcas=[3,4], fixed_cms=[1,2,4], fixed_sketch_size=4194304)
    
    plt.show()

if __name__ == '__main__':
    plot_all_alpha()        
