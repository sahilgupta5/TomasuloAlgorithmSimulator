# Tomasulo Algorithm Simulator

Wiki page for this: https://en.wikipedia.org/wiki/Tomasulo_algorithm

Tomasulo's algorithm simulator displays the famous computer architecture hardware algorithm for dynamic scheduling of instructions that allows out-of-order execution, designed to efficiently utilize multiple execution units.

1. Vary	 the following parameters:	

FOR the gcc.100k.trace:

• Number of FUs of each type (k0, k1, k2) of either 1 or 2 or 3 units each.	
Keeping everything else constant, let’s see how increasing/decreasing k0, k1 and k2 affect the runtime of the program.

K0 varies
For instance: F/N = 2, m = 2, k0 = varies, k1 = 1, k2 = 1, R = 8
	
	Average Instruction/Cycle	Increase/Decrease
K0 = 1	0.600424	
K0 = 2	0.930960	Increased from K0 = 1
K0 = 3	0.977632	Increased from K0 = 2

K1 varies
For instance: F/N = 2, m = 2, k0 = 3, k1 = varies, k2 = 1, R = 8
	
	Average Instruction/Cycle	Increase/Decrease
K1 = 1	0.977632	
K1 = 2	1.030089	Increased from K1 = 1
K1 = 3	1.030089	Became constant or same as K2 = 2

K2 varies
For instance: F/N = 2, m = 2, k0 = 3, k1 = 1, k2 = varies, R = 8
	
	Average Instruction/Cycle	Increase/Decrease
K2 = 1	0.977632	
K2 = 2	1.035615	Increased from k2 = 1
K2 = 3	1.039069	Increased from k2 = 2


K1 & K2 varies
For instance: F/N = 2, m = 2, k0 = varies, k1 = varies, k2 = 3, R = 8
Using the same approach as above, keeping k2 and everything else constant, increasing both k1 and k2 results in increasing IPC but with diminishing returns. Decreasing k0 and increasing k1 decreases IPC significantly (0.6 from 1.04). Increasing k0 and decreasing k1 decreases IPC but less significantly than before (1.03 from 1.04).

2. To see this first let’s use the best hardware possible by using max resources available. 
For instance: F = 8, M = 8, k0 = 3, k1 = 3, k2 = 3 and R = 128.
IPC = 3.74911
F  = 4 is best because from F = 8 there is not much decrease in IPC but F = 2 decreases significantly. R = 128 and R = 32 decrease IPC significantly. So, R = 128 is best. M = 4 is same as M = 8 but the IPC decreases for M = 2. Hence, M = 4 is the best after varying the values since M = 8 is very close to M = 4 but there is a significant decrease of IPC for M = 2. Similarly, K0 = 2 is the best, K1 = 2 is the best, k2 = 2 is the best since the IPC does not vary when we have 3 functional units.

1. Vary	 the following parameters:	

FOR the gob.100k.trace:

To see this first let’s use the best hardware possible by using max resources available. 
For instance: F = 8, M = 8, k0 = 3, k1 = 3, k2 = 3 and R = 128.

2. 
IPC = 3.132440
F  = 4 is best because from F = 8 there is not much decrease in IPC but F = 2 decreases significantly. R = 128 and R = 32 decrease IPC significantly. So, R = 128 is best.  M = 8 is the best since there is significant linear decrease when m = 4 and m = 2 in terms of IPC. Similarly, K0 = 3 is the best since IPC is the best when 3 Functional units are present, K1 = 1 is the best since having 1 FU does not decrease IPC (IPC = 2.9) much when we have 3 FUs (IPC = 3.13). Finally, K2 = 2 is the best as IPC decreases significantly from K3 = 2 (IPC = 3.1) to K3 = 1 (IPC = 2.4).

FOR the gob.100k.trace:

To see this first let’s use the best hardware possible by using max resources available. 
For instance: F = 8, M = 8, k0 = 3, k1 = 3, k2 = 3 and R = 128.

IPC = 3.132440
F  = 4 is best because from F = 8 there is not much decrease in IPC but F = 2 decreases significantly. R = 128 and R = 32 decrease IPC significantly. So, R = 128 is best.  M = 8 is the best since there is significant linear decrease when m = 4 and m = 2 in terms of IPC. Similarly, K0 = 3 is the best since IPC is the best when 3 Functional units are present, K1 = 1 is the best since having 1 FU does not decrease IPC (IPC = 2.9) much when we have 3 FUs (IPC = 3.13). Finally, K2 = 2 is the best as IPC decreases significantly from K3 = 2 (IPC = 3.1) to K3 = 1 (IPC = 2.4).

FOR the mcf.100k.trace:

To see this first let’s use the best hardware possible by using max resources available. 
For instance: F = 8, M = 8, k0 = 3, k1 = 3, k2 = 3 and R = 128.

IPC = 4.122691
F  = 4 is best because from F = 8 there is not much decrease in IPC but F = 2 decreases significantly. R = 128 and R = 32 decrease IPC significantly. So, R = 128 is best.  M = 8 is the best since there is significant linear decrease when m = 4 and m = 2 in terms of IPC. Similarly, K0 = 3 is the best since IPC is the best when 3 Functional units are present, K1 = 1 is the best since having 1 FU does not decrease IPC (IPC = 2.9) much when we have 3 FUs (IPC = 3.13). Finally, K2 = 2 is the best as IPC decreases significantly from K3 = 2 (IPC = 4.04) to K3 = 2 (IPC = 3.0).

FOR the hmmer.100k.trace:

To see this first let’s use the best hardware possible by using max resources available. 
For instance: F = 8, M = 8, k0 = 3, k1 = 3, k2 = 3 and R = 128.

IPC = 2.9497
F  = 4 is best because from F = 8 there is not much decrease in IPC but F = 2 decreases significantly. R = 128 and R = 32 decrease IPC significantly. So, R = 128 is best.  M = 4 (IPC = 2.4) is the best since there is significant linear decrease when m = 2 but not a huge decrease when M = 8 in terms of IPC. Similarly, K0 = 3 is the best since IPC is the best when 3 Functional units are present, K1 = 1 is the best since having 1 FU does not decrease IPC (IPC = 2.9) much when we have 3 FUs (IPC = 3.13). Finally, K2 = 2 is the best as IPC decreases significantly from K3 = 2 (IPC = 4.04) to K3 = 2 (IPC = 3.0).
