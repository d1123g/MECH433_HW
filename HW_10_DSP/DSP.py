import csv
import matplotlib.pyplot as plt # for plotting
import numpy as np # for numerical operations

t_A = [] # column 0
data_A = [] # column 1

t_B = [] # column 0
data_B = [] # column 1  

t_C = [] # column 0
data_C = [] # column 1

t_D = [] # column 0
data_D = [] # column 1


#--------------------------------------------------------------
with open('sigA.csv') as f:
    # open the csv file
    reader = csv.reader(f)
    for row in reader:
        # read the rows 1 one by one
        t_A.append(float(row[0])) # leftmost column
        data_B.append(float(row[1])) # second column

# plotting
plt.plot(t_A, data_A, label='Signal A')
plt.xlabel('Time (s)')
plt.ylabel('Amplitude')
plt.title('Signal A')

total_time_A = t_A[-1] - t_A[0]
number_of_samples_A = len(data_A)
sampling_rate_A = number_of_samples_A / total_time_A
Fs_A = sampling_rate_A

# for i in range(len(t)):
#     # print the data to verify it was read
#     print(str(t[i]) + ", " + str(data1[i]))

#--------------------------------------------------------------
with open('sigB.csv') as f:
    # open the csv file
    reader = csv.reader(f)
    for row in reader:
        # read the rows 1 one by one
        t_B.append(float(row[0])) # leftmost column
        data_B.append(float(row[1])) # second column

# plotting
plt.plot(t_B, data_B, label='Signal B')
plt.xlabel('Time (s)')
plt.ylabel('Amplitude')
plt.title('Signal B')

total_time_B = t_B[-1] - t_B[0]
number_of_samples_B = len(data_B)
sampling_rate_B = number_of_samples_B / total_time_B
Fs = sampling_rate_B

# for i in range(len(t)):
#     # print the data to verify it was read
#     print(str(t[i]) + ", " + str(data1[i]))


#--------------------------------------------------------------
with open('sigC.csv') as f:
    # open the csv file
    reader = csv.reader(f)
    for row in reader:
        # read the rows 1 one by one
        t_C.append(float(row[0])) # leftmost column
        data_C.append(float(row[1])) # second column


# plotting
plt.plot(t_C, data_C, label='Signal C')
plt.xlabel('Time (s)')
plt.ylabel('Amplitude')
plt.title('Signal C')

total_time_C = t_C[-1] - t_C[0]
number_of_samples_C = len(data_C)
sampling_rate_C = number_of_samples_C / total_time_C
Fs_C = sampling_rate_C
Ts_C = 1 / Fs_C # sampling period
# for i in range(len(t)):
#     # print the data to verify it was read
#     print(str(t[i]) + ", " + str(data1[i]))


#------------------------------------------------------------
with open('sigD.csv') as f:
    # open the csv file
    reader = csv.reader(f)
    for row in reader:
        # read the rows 1 one by one
        t_D.append(float(row[0])) # leftmost column
        data_D.append(float(row[1])) # second column


# plotting
plt.plot(t_D, data_D, label='Signal D')
plt.xlabel('Time (s)')   
plt.ylabel('Amplitude')
plt.title('Signal D')


total_time_D = t_D[-1] - t_D[0]
number_of_samples_D = len(data_D)
sampling_rate_D = number_of_samples_D / total_time_D
Fs_D = sampling_rate_D
Ts_D = 1 / Fs_D # sampling period

# for i in range(len(t)):
#     # print the data to verify it was read
#     print(str(t[i]) + ", " + str(data1[i]))