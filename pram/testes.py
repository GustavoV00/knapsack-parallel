import os
import re
import numpy as np
from tabulate import tabulate

def run_code(command, seq_time, paral_time, totalParal, flag):
    
    os.system(command)
    with open("results/results.out", "r") as f:
        # os.system("cat results/results.out")
        if(flag == 0):
            test = f.readline()
            seq_time.append(test)
            seq_time = map(lambda s: s.strip(), seq_time)
            print(seq_time)
        else:
            content  = f.readlines()
            newLst = [x[:-1] for x in content]
            # newLst2 = []
            # for i in newLst:
            #     newLst2.append(re.sub("[^0-9]", "", i))

            seq_time.append(float(newLst[0]))
            paral_time.append(float(newLst[1]))
            totalParal.append(float(newLst[2]))

            print(seq_time)
            print(paral_time)
            print(totalParal)
            print("\n")


def generateTests():
    weighth = 1
    inputs = []
    for i in range(10):
        nth = f"{(i*30)+1}"
        wth = f"{weighth*5}"
        input = nth + ' ' + wth

        os.system(f"python3 generator.py {input}")
        inputs.append(f"problems/{nth}_{wth}")
        weighth += 1

    return inputs


def testSeqCode(seqInputs, inputs):
    for item in inputs:
        run_code(f"./knapsack-serial < {item} > results/results.out", seqInputs, [], [], 0)

def testParalCode(seqPartOfParal, paralPart, totalParal, inputs):
    for item in inputs:
        run_code(f"./knapsack-parallel < {item} > results/results.out", seqPartOfParal, paralPart, totalParal, 1)

 
def amdahl(percent, threads):

    beta =  percent / 100
    result = 1 / beta + ((1 - beta) / threads)
    print(result, "\n")

    return result

def speedupTable(seqInputs, paralInputs, speedupList):
    arrAux = []
    
    i = 0
    while(i < len(seqInputs)):
        print("swq:", seqInputs[i])
        print("paral: ", paralInputs[i])
        print("lolololo\n")
        result = float(seqInputs[i]) / float(paralInputs[i])
        arrAux.append(result)
        i += 1


    print(arrAux)
    speedupList = np.append(speedupList, arrAux, axis=0)
    return speedupList

def main():
    # speedup_list = []
    seqInputs = []

    seqPartOfParal = []
    paralInputs = []
    totalParal = []

    speedupList = np.array([])

    # seq_porcent = []
    # paral_porcent = []
    os.system("make")

    inputs = generateTests()
    # for _ in range(10):
    #     testSeqCode(seqInputs, inputs)
    #     testParalCode(seqPartOfParal, paralInputs, totalParal, inputs)
    #     speedupList = speedupTable(seqInputs, totalParal, speedupList)
    # print(speedupList)


    # print(seqPartOfParal, '\n')
    # print(paralInputs, '\n')
    # print(totalParal, '\n')
    


    os.system("make clean")


    return;



if __name__ == "__main__":
    main()
