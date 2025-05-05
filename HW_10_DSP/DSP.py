import csv
import matplotlib.pyplot as plt
import numpy as np
import os
    

# FIR Filter Design
# we can input in the weights later
def fir_low_pass_filter(data, h):
    filtered_data = np.zeros(len(data)) # Initialize filtered data array
    for i in range(len(data)): # Loop through each data point
        for j in range(len(h)): # Loop through filter coefficients
            if i - j >= 0:
                filtered_data[i] += h[j] * data[i - j] # Convolve filter with data
    return filtered_data

# Moving average low-pass filter
def moving_average_filter(data, X):
    filtered_data = np.zeros(len(data)) # Initialize filtered data array
    for i in range(len(data)): # Loop through each data point
        if i < X:
            filtered_data[i] = np.mean(data[:i+1]) # Average from start to current point
        else:
            filtered_data[i] = np.mean(data[i-X:i]) # Average over the last X points
    return filtered_data

# IIR low-pass filter
def iir_low_pass_filter(data, A, B):
    filtered_data = np.zeros(len(data)) # Initialize filtered data array
    filtered_data[0] = data[0]  # start from first value
    for i in range(1, len(data)): # Loop through each data point
        filtered_data[i] = A * filtered_data[i-1] + B * data[i] # Apply IIR filter formula
    return filtered_data

# Plotting function for both time and frequency domain
def plot_signal_and_fft(t, data, filtered_data, title_label):
    save_dir = r"C:\Users\gonza\Desktop\Advanced_Mechatronics\rep_HWs\MECH433_HW\HW_10_DSP\plots"
    if not os.path.exists(save_dir):
        os.makedirs(save_dir)
    total_time = t[-1] - t[0]
    n = len(data) # Number of data points
    Fs = n / total_time # Sampling frequency
    print("Sampling frequency: ", Fs)
    T = n / Fs # Sampling period
    k = np.arange(n)
    frq = k / T # Frequency range
    frq = frq[range(int(n / 2))] # One side frequency range

    Y = np.fft.fft(data) / n # FFT of original data
    Y_filtered = np.fft.fft(filtered_data) / n # FFT of filtered data
    Y = Y[range(int(n / 2))] # One side FFT
    Y_filtered = Y_filtered[range(int(n / 2))] # One side FFT

    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 6)) # Create subplots

    ax1.plot(t, data, 'k', label='Unfiltered') # Plot original data
    ax1.plot(t, filtered_data, 'r', label='Filtered') # Plot filtered data
    ax1.set_xlabel('Time (s)')
    ax1.set_ylabel('Amplitude')
    ax1.set_title(f'Signal and Filtered Signal ({title_label})')
    ax1.legend()

    ax2.loglog(frq, abs(Y), 'k', label='Unfiltered FFT') # Plot FFT of original data
    ax2.loglog(frq, abs(Y_filtered), 'r', label='Filtered FFT') # Plot FFT of filtered data
    ax2.set_xlabel('Frequency (Hz)')
    ax2.set_ylabel('|Y(freq)|')
    ax2.set_title(f'FFT Comparison ({title_label})')
    ax2.legend()
    # Save the figure always
    safe_title = title_label.replace(" ", "_").replace("(", "").replace(")", "").replace("=", "")
    fig_path = os.path.join(save_dir, f"{safe_title}.png")
    plt.savefig(fig_path, dpi=300)
    print(f"Saved plot to {fig_path}")

# General processing function for either filter
def process_and_plot(csv_file, filter_type="moving", X=None, A=None, B=None, h=None,
                     cutoff=None, bandwidth=None, window=None):
    t, data = [], []

    with open(csv_file) as f:
        reader = csv.reader(f)
        for row in reader:
            t.append(float(row[0]))
            data.append(float(row[1]))
    t = np.array(t)
    data = np.array(data)
    
    # Build label based on filter type
    if filter_type == "moving":
        filtered_data = moving_average_filter(data, X)
        label = f"Moving Avg (X={X})"
    
    elif filter_type == "iir":
        filtered_data = iir_low_pass_filter(data, A, B)
        label = f"IIR (A={A}, B={B})"
    
    elif filter_type == "fir":
        if h is None:
            raise ValueError("FIR filter selected, but no coefficients 'h' provided.")
        filtered_data = fir_low_pass_filter(data, h)
        label = f"FIR (len(h)={len(h)})"

        # Append FIR for cutoff, bandwidth, and window to label
        meta = []
        if cutoff is not None:
            meta.append(f"Cutoff={cutoff}")
        if bandwidth is not None:
            meta.append(f"BW={bandwidth}")
        if window is not None:
            meta.append(f"Window={window}")
        if meta:
            label += " [" + ", ".join(meta) + "]"
    
    else:
        raise ValueError("Unknown filter type.")

    plot_signal_and_fft(t, data, filtered_data, f"{csv_file} - {label}")



# --- Run filters on all signals ---
# Moving average filtering
process_and_plot('sigA.csv', filter_type="moving", X=25) # good for sigA
process_and_plot('sigB.csv', filter_type="moving", X=20) 
process_and_plot('sigC.csv', filter_type="moving", X=20) # good for sigC
process_and_plot('sigD.csv', filter_type="moving", X=20) # good for sigD

# IIR filtering
process_and_plot('sigA.csv', filter_type="iir", A=0.95, B=0.05)
process_and_plot('sigB.csv', filter_type="iir", A=.95, B=0.05)
process_and_plot('sigC.csv', filter_type="iir", A=0.9, B=0.1) # good for sigC
process_and_plot('sigD.csv', filter_type="iir", A=0.9, B=0.1) # good for sigD

# The window type will be between Rectangular, Hamming, Blackman, and Kaiser

# Signal A cutoff is 1000 bandwidth is 4000, sampling rate is 10000 Rectangular
h = [
    0.325843353326760232,
    0.348313293346479480,
    0.325843353326760232,
]
process_and_plot("sigA.csv", filter_type="fir", h=h, cutoff=1000, bandwidth=4000, window="Rectangular")

# Signal A cutoff is 1000 bandwidth is 1000, sampling rate is 10000 Rectangular
h = [
    0.000000000000000007,
    0.039899882278863805,
    0.086079154232428276,
    0.129118731348642435,
    0.159599529115455163,
    0.170605406049220421,
    0.159599529115455163,
    0.129118731348642435,
    0.086079154232428276,
    0.039899882278863805,
    0.000000000000000007,
]
process_and_plot("sigA.csv", filter_type="fir", h=h, cutoff=1000, bandwidth=1000, window="Rectangular")

# Signal A cutoff is 2500 bandwidth is 1000, sampling rate is 10000 Rectangular
h = [
    0.060530312237274071,
    -0.000000000000000019,
    -0.100883853728790121,
    0.000000000000000019,
    0.302651561186370377,
    0.475403960610291443,
    0.302651561186370377,
    0.000000000000000019,
    -0.100883853728790121,
    -0.000000000000000019,
    0.060530312237274071,
]
process_and_plot("sigA.csv", filter_type="fir", h=h, cutoff=2500, bandwidth=1000, window="Rectangular")

# Signal B cutoff is 330 bandwidth is 825, sampling rate is 3300 Hamming
h = [
    -0.002719748296486631,
    0.000000000000000001,
    0.015808536973328534,
    0.059408709991559894,
    0.127068629704169794,
    0.191410109532370587,
    0.218047524190115638,
    0.191410109532370615,
    0.127068629704169822,
    0.059408709991559908,
    0.015808536973328534,
    0.000000000000000001,
    -0.002719748296486631,
]
process_and_plot("sigB.csv", filter_type="fir", h=h, cutoff=330, bandwidth=825, window="Hamming")

# Signal B cutoff is 330 bandwidth is 330, sampling rate is 3300 Hamming
h = [
    0.000000000000000001,
    0.001201426375882775,
    0.002784327847982891,
    0.004227316085446900,
    0.003942763412776409,
    -0.000000000000000002,
    -0.008256777292656481,
    -0.018583210130235957,
    -0.025389820435008297,
    -0.021235308698378592,
    0.000000000000000006,
    0.039588112570909842,
    0.091888877020527615,
    0.145099081031584087,
    0.184902878561820999,
    0.199660667298695443,
    0.184902878561820999,
    0.145099081031584087,
    0.091888877020527657,
    0.039588112570909856,
    0.000000000000000006,
    -0.021235308698378599,
    -0.025389820435008315,
    -0.018583210130235967,
    -0.008256777292656481,
    -0.000000000000000002,
    0.003942763412776408,
    0.004227316085446901,
    0.002784327847982890,
    0.001201426375882775,
    0.000000000000000001,
]
process_and_plot("sigB.csv", filter_type="fir", h=h, cutoff=330, bandwidth=330, window="Hamming")



# Signal B cutoff is 825 bandwidth is 330, sampling rate is 3300 Hamming
h = [
    -0.001700396903673608,
    0.000000000000000002,
    0.002937331570890679,
    -0.000000000000000003,
    -0.006730091366404412,
    0.000000000000000006,
    0.014093887903991933,
    -0.000000000000000010,
    -0.026785035820053850,
    0.000000000000000013,
    0.049098960593575401,
    -0.000000000000000017,
    -0.096938332776300790,
    0.000000000000000019,
    0.315619563324482266,
    0.500808226946984569,
    0.315619563324482266,
    0.000000000000000019,
    -0.096938332776300817,
    -0.000000000000000017,
    0.049098960593575422,
    0.000000000000000013,
    -0.026785035820053871,
    -0.000000000000000010,
    0.014093887903991936,
    0.000000000000000006,
    -0.006730091366404410,
    -0.000000000000000003,
    0.002937331570890678,
    0.000000000000000002,
    -0.001700396903673608,
]
process_and_plot("sigB.csv", filter_type="fir", h=h, cutoff=825, bandwidth=330, window="Hamming")


# Signal C cutoff is 625 bandwidth is 250, sampling rate is 2500 Blackman
h = [
    0.000000000000000000,
    0.000023261921107576,
    0.000061432444651041,
    0.000000000000000000,
    -0.000291447722004781,
    -0.000814396670401605,
    -0.001304722428486792,
    -0.001226359116783539,
    0.000000000000000001,
    0.002546027087308252,
    0.005703125628112526,
    0.007735724529073834,
    0.006376352590531455,
    -0.000000000000000003,
    -0.010922498655864386,
    -0.022853350749001102,
    -0.029476489738831897,
    -0.023569701466005240,
    0.000000000000000006,
    0.041351205028448933,
    0.094174711313959839,
    0.146802637928974300,
    0.185679937824334729,
    0.200009100501753745,
    0.185679937824334729,
    0.146802637928974300,
    0.094174711313959825,
    0.041351205028448947,
    0.000000000000000006,
    -0.023569701466005240,
    -0.029476489738831897,
    -0.022853350749001120,
    -0.010922498655864390,
    -0.000000000000000003,
    0.006376352590531451,
    0.007735724529073836,
    0.005703125628112531,
    0.002546027087308253,
    0.000000000000000001,
    -0.001226359116783541,
    -0.001304722428486792,
    -0.000814396670401606,
    -0.000291447722004781,
    0.000000000000000000,
    0.000061432444651041,
    0.000023261921107576,
    0.000000000000000000,
]
process_and_plot("sigC.csv", filter_type="fir", h=h, cutoff=625, bandwidth=250, window="Blackman")


# Signal C cutoff is 250 bandwidth is 250, sampling rate is 2500 Blackman
h = [
    0.000000000000000000,
    0.000023261921107576,
    0.000061432444651041,
    0.000000000000000000,
    -0.000291447722004781,
    -0.000814396670401605,
    -0.001304722428486792,
    -0.001226359116783539,
    0.000000000000000001,
    0.002546027087308252,
    0.005703125628112526,
    0.007735724529073834,
    0.006376352590531455,
    -0.000000000000000003,
    -0.010922498655864386,
    -0.022853350749001102,
    -0.029476489738831897,
    -0.023569701466005240,
    0.000000000000000006,
    0.041351205028448933,
    0.094174711313959839,
    0.146802637928974300,
    0.185679937824334729,
    0.200009100501753745,
    0.185679937824334729,
    0.146802637928974300,
    0.094174711313959825,
    0.041351205028448947,
    0.000000000000000006,
    -0.023569701466005240,
    -0.029476489738831897,
    -0.022853350749001120,
    -0.010922498655864390,
    -0.000000000000000003,
    0.006376352590531451,
    0.007735724529073836,
    0.005703125628112531,
    0.002546027087308253,
    0.000000000000000001,
    -0.001226359116783541,
    -0.001304722428486792,
    -0.000814396670401606,
    -0.000291447722004781,
    0.000000000000000000,
    0.000061432444651041,
    0.000023261921107576,
    0.000000000000000000,
]
process_and_plot("sigC.csv", filter_type="fir", h=h, cutoff=250, bandwidth=250, window="Blackman")


# Signal C cutoff is 250 bandwidth is 625, sampling rate is 2500 Blackman
h = [
    0.000000000000000000,
    -0.000452073562033652,
    -0.002297937600926329,
    -0.004234304945305980,
    0.000000000000000002,
    0.021089216963617326,
    0.066404416488449372,
    0.129158815530706156,
    0.185878795133768981,
    0.208906143983448089,
    0.185878795133768981,
    0.129158815530706211,
    0.066404416488449400,
    0.021089216963617326,
    0.000000000000000002,
    -0.004234304945305989,
    -0.002297937600926331,
    -0.000452073562033653,
    0.000000000000000000,
]
process_and_plot("sigC.csv", filter_type="fir", h=h, cutoff=250, bandwidth=625, window="Blackman")




# Signal D cutoff is 40 bandwidth is 100, sampling rate is 400 Kaiser
h = [
    0.000000000000000001,
    0.017202553324110307,
    0.061713642877670657,
    0.127181981478889067,
    0.187685579449425460,
    0.212432485739809079,
    0.187685579449425460,
    0.127181981478889150,
    0.061713642877670623,
    0.017202553324110307,
    0.000000000000000001,
]
process_and_plot("sigD.csv", filter_type="fir", h=h, cutoff=40, bandwidth=100, window="Kaiser")



# Signal D cutoff is 40 bandwidth is 40, sampling rate is 400 Kaiser
h = [
    0.000000000000000001,
    0.017202553324110307,
    0.061713642877670657,
    0.127181981478889067,
    0.187685579449425460,
    0.212432485739809079,
    0.187685579449425460,
    0.127181981478889150,
    0.061713642877670623,
    0.017202553324110307,
    0.000000000000000001,
]
process_and_plot("sigD.csv", filter_type="fir", h=h, cutoff=40, bandwidth=40, window="Kaiser")


# Signal D cutoff is 100 bandwidth is 40, sampling rate is 400 Kaiser
h = [
    -0.000000000000000003,
    -0.006516192064016200,
    0.000000000000000006,
    0.014280311428581572,
    -0.000000000000000010,
    -0.027151498821544103,
    0.000000000000000013,
    0.049507622881582232,
    -0.000000000000000017,
    -0.097347574494857106,
    0.000000000000000019,
    0.316324531370552109,
    0.501805599399402946,
    0.316324531370552109,
    0.000000000000000019,
    -0.097347574494857106,
    -0.000000000000000017,
    0.049507622881582232,
    0.000000000000000013,
    -0.027151498821544103,
    -0.000000000000000010,
    0.014280311428581572,
    0.000000000000000006,
    -0.006516192064016197,
    -0.000000000000000003,
]
process_and_plot("sigD.csv", filter_type="fir", h=h, cutoff=100, bandwidth=40, window="Kaiser")


plt.show()
