import os
import matplotlib.pyplot as plt
import statistics

ORCAS = 'ORCAS'
CMS = 'CMS'

SKETCHES = [ORCAS, CMS]
THRESHOLDS = [0.0001, 0.000178, 0.000316, 0.000562, 0.001, 0.00178, 0.00316, 0.00562, 0.01]
ALPHA = [0.6, 0.8, 1.0, 1.2, 1.5]

def get_all_values(hh_data, fixed_orcas, fixed_cms, fixed_sketch_size, fixed_threshold):

    hh_dict = {}
    
    for sketch in SKETCHES:

        if sketch == ORCAS:
            for array_counter in fixed_orcas:
                key = ORCAS + '-' + str(array_counter)

                hh_dict[key] = {}

                sketch_hh_data = hh_data[sketch]
                for alpha in sketch_hh_data:
                    for line in sketch_hh_data[alpha]:
                        split_line = line.split('\t')
                        sketch_size = int(split_line[3])
                        n_arrays = int(split_line[5])
                        n_array_counters = int(split_line[7])
                        rel_error = 0
                        j = 11
                        for threshold in THRESHOLDS:
                            if threshold == fixed_threshold:
                                rel_error = float(split_line[j])
                                break
                            j += 4

                        if (n_array_counters == array_counter) and (sketch_size == fixed_sketch_size):
                            if alpha not in hh_dict[key]:
                                hh_dict[key][alpha] = []
                            hh_dict[key][alpha].append(rel_error)

        elif sketch == CMS:
            for row_number in fixed_cms:
                key = CMS + '-' + str(row_number)

                hh_dict[key] = {}

                sketch_hh_data = hh_data[sketch]
                for alpha in sketch_hh_data:
                    for line in sketch_hh_data[alpha]:
                        split_line = line.split('\t')
                        width = int(split_line[3])
                        n_rows = int(split_line[5])
                        rel_error = 0
                        j = 9
                        for threshold in THRESHOLDS:
                            if threshold == fixed_threshold:
                                rel_error = float(split_line[j])
                                break
                            j += 4

                        if (n_rows == row_number) and ((n_rows*width) == fixed_sketch_size):
                            if alpha not in hh_dict[key]:
                                hh_dict[key][alpha] = []
                            hh_dict[key][alpha].append(rel_error)

    return hh_dict

def get_median(hh_dict_all):

    hh_dict = {}

    for key in hh_dict_all:
        hh_dict[key] = []

        for alpha in hh_dict_all[key]:
            med = statistics.median(hh_dict_all[key][alpha])
            hh_dict[key].append((alpha, med))

    return hh_dict

def plot_speed_and_error(fixed_orcas:int, fixed_cms:int, fixed_sketch_size=None, fixed_threshold=None): # ORCAS array counters, CMS row numbers

    hh_data = {}

    for sketch in SKETCHES:
        hh_data[sketch] = {}

    for alpha in ALPHA:
        hh_data[ORCAS][alpha] = []

        alpha_str = str(alpha)[0] + '-' + str(alpha)[-1]
        alpha_folder_name = '../../ORCAS-test-results/alpha_' + alpha_str + '/counters_mode/'

        for fn in os.listdir(alpha_folder_name):
            indiv_data = open(alpha_folder_name + fn).readlines()
            indiv_data = indiv_data[1:] # remove zipf generation line
            if 'final_error' in fn:
                hh_data[ORCAS][alpha] = hh_data[ORCAS][alpha] + indiv_data

        hh_data[CMS][alpha] = []

        alpha_str = str(alpha)[0] + '-' + str(alpha)[-1]
        alpha_folder_name = '../../../baseline-results/cms/alpha_' + alpha_str + '/'

        for fn in os.listdir(alpha_folder_name):
            indiv_data = open(alpha_folder_name + fn).readlines()
            if 'final_error' in fn:
                hh_data[CMS][alpha] = hh_data[CMS][alpha] + indiv_data

    hh_dict_all = get_all_values(hh_data, fixed_orcas, fixed_cms, fixed_sketch_size, fixed_threshold)
    hh_dict = get_median(hh_dict_all)

    fig, (hh_ax) = plt.subplots(1, 1)

    for key in hh_dict:
        key_label = key.split('-')[0] + ': ' + key.split('-')[1]
        hh_x = [alpha for (alpha, rel_error) in hh_dict[key]]
        hh_y = [rel_error for (alpha, rel_error) in hh_dict[key]]
        hh_ax.plot(hh_x, hh_y, label=str(key_label))

    N = hh_data[ORCAS][0.6][0].split('\t')[1]
    fig_title = 'N = ' + N + ' | ORCAS config: array counter | CMS config: row number'
    if fixed_sketch_size is not None:
        fig_title = fig_title + ' | fixed sketch size = ' + str(fixed_sketch_size)
    # fig.suptitle(fig_title, fontsize=14)

    hh_ax.set_xlabel('Alpha', fontsize=22)
    hh_ax.set_ylabel('ARE', fontsize=22)
    hh_ax.tick_params(axis='both', which='both', labelsize=20, width=1, length=4)
    hh_ax.set_xscale('log',base=10)
    hh_ax.set_yscale('log',base=10)
    hh_ax.legend(loc="upper right", prop={"size":20}, ncol=len([key for key in hh_dict]), bbox_to_anchor=(1.1, 1.14))

def plot_all_alpha():

    plot_speed_and_error(fixed_orcas=[3,4], fixed_cms=[1,2,4], fixed_sketch_size=65536, fixed_threshold=0.000316)
    
    plt.show()

if __name__ == '__main__':
    plot_all_alpha()        
