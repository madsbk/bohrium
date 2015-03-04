#!/usr/bin/env bash
echo "NumPy"
OMP_NUM_THREADS=1 BH_VE_CPU_JIT_FUSION=1 BH_VE_CPU_JIT_DUMPSRC=1 python ~/bohrium/benchmark/python/lbm_3d.py --size=150*150*150*10 --bohrium=False
echo "SIJ"
OMP_NUM_THREADS=1 BH_VE_CPU_JIT_FUSION=0 BH_VE_CPU_JIT_DUMPSRC=1 python ~/bohrium/benchmark/python/lbm_3d.py --size=150*150*150*10 --bohrium=True
OMP_NUM_THREADS=1 BH_VE_CPU_JIT_FUSION=0 BH_VE_CPU_JIT_DUMPSRC=1 python ~/bohrium/benchmark/python/lbm_3d.py --size=150*150*150*10 --bohrium=True > sij.txt
echo "Fusion"
OMP_NUM_THREADS=1 BH_VE_CPU_JIT_FUSION=1 BH_VE_CPU_JIT_DUMPSRC=1 python ~/bohrium/benchmark/python/lbm_3d.py --size=150*150*150*10 --bohrium=True
OMP_NUM_THREADS=1 BH_VE_CPU_JIT_FUSION=1 BH_VE_CPU_JIT_DUMPSRC=1 python ~/bohrium/benchmark/python/lbm_3d.py --size=150*150*150*10 --bohrium=True > fused.txt

