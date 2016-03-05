BH_SRC=$(realpath ../../..)
CPU=$(BH_SRC)/ve/cpu
BDIR=/tmp/bhbuild_cpu/
#PAR_LVL=-j$(shell expr $(shell nproc) / 2)
PAR_LVL=-j1
BH_PYTHON=python

# Excluding test to preserve battery... I am one a plane..

# PASS
TEST_EXCLUDE=--exclude=test_primitives.py --exclude=test_accumulate.py --exclude=test_specials.py --exclude=test_accumulate.py
TEST_EXCLUDE+= --exclude=test_reduce.py --exclude=test_bohrium.py --exclude=test_ndstencil.py --exclude=test_emptiness.py --exclude=test_array_create.py
TEST_EXCLUDE+= --exclude-test=test_black_scholes --exclude-test=test_gameoflife --exclude-test=test_gauss --exclude-test=test_heat_equation --exclude-test=test_gauss_20x20 --exclude-test=test_heat_equation_fixed --exclude-test=test_lu --exclude-test=test_mxmul --exclude-test=test_point27 --exclude-test=test_rosenbrock --exclude-test=test_shallow_water --exclude-test=test_snakes_and_ladders --exclude-test=test_synth --exclude-test=test_wireworld  --exclude-test=test_nbody --exclude-test=test_jacobi --exclude-test=test_jacobi_fixed  --exclude-test=test_knn_naive   --exclude-test=test_xraysim
TEST_EXCLUDE+= --exclude=test_views.py --exclude=test_matmul.py --exclude=test_sign.py --exclude=test_types.py

# FAIL
TEST_EXCLUDE+= --exclude-test=test_idl_init --exclude-test=test_idl_init_1loop
#TEST_EXCLUDE+=  --exclude-test=test_sor

#TEST=--test=test_heat_equation
DUMPSRC=1

BUILD_OPTIONS=

gnu:
	$(eval CC := gcc)
	$(eval CXX := g++)

gnu49:
	$(eval CC := gcc-4.9)
	$(eval CXX := g++)

gnu53:
	$(eval CC := gcc-5.3)
	$(eval CXX := g++)

pgi:
	$(eval CC := pgcc)
	$(eval CXX := g++)

path:
	$(eval CC := pathcc)
	$(eval CXX := g++)

intel:
	$(eval CC := icc)
	$(eval CXX := g++)

clang:
	$(eval CC := clang)
	$(eval CXX := clang++)

release_on:
	$(eval BUILD_OPTIONS := $(BUILD_OPTIONS) -DCMAKE_BUILD_TYPE=Release)

cil_off:
	$(eval BUILD_OPTIONS := $(BUILD_OPTIONS) -DBENCHMARK_CIL=OFF -DBRIDGE_CIL=OFF -DTEST_CIL=OFF -DBRIDGE_NUMCIL=OFF )

cluster_off:
	$(eval BUILD_OPTIONS := $(BUILD_OPTIONS) -DVEM_CLUSTER=OFF -DVEM_PROXY=OFF )

fun_off:
	$(eval BUILD_OPTIONS := $(BUILD_OPTIONS) -DEXT_VISUALIZER=OFF )

gpu_off:
	$(eval BUILD_OPTIONS := $(BUILD_OPTIONS) -DVE_GPU=OFF )

cpu_debug_on:
	$(eval BUILD_OPTIONS := $(BUILD_OPTIONS) -DVE_CPU_DEBUGGING=ON)

cpu_profiling_on:
	$(eval BUILD_OPTIONS := $(BUILD_OPTIONS) -DVE_CPU_PROFILING=ON)

fuser_gentle_off:
	$(eval BUILD_OPTIONS := $(BUILD_OPTIONS) -DFUSER_GENTLE=OFF)

fuser_greedy_off:
	$(eval BUILD_OPTIONS := $(BUILD_OPTIONS) -DFUSER_GREEDY=OFF)

fuser_optimal_off:
	$(eval BUILD_OPTIONS := $(BUILD_OPTIONS) -DFUSER_OPTIMAL=OFF)

fuser_singleton_off:
	$(eval BUILD_OPTIONS := $(BUILD_OPTIONS) -DFUSER_SINGLETON=OFF)

fuser_topological_only: fuser_greedy_off fuser_optimal_off fuser_singleton_off

custom_boost_154:
	$(eval BUILD_OPTIONS := $(BUILD_OPTIONS) -DBOOST_ROOT=$(HOME)/tools/boost-1.54.0 -DBoost_INCLUDE_DIRS=$(HOME)/tools/boost-1.54.0/include -DBoost_LIBRARY_DIRS=$(HOME)/tools/boost-1.54.0/lib -DBoost_NO_SYSTEM_PATHS=ON -DBoost_NO_BOOST_CMAKE=ON )

custom_boost_158:
	$(eval BUILD_OPTIONS := $(BUILD_OPTIONS) -DBOOST_ROOT=$(HOME)/tools/boost-1.58.0 -DBoost_INCLUDE_DIRS=$(HOME)/tools/boost-1.58.0/include -DBoost_LIBRARY_DIRS=$(HOME)/tools/boost-1.58.0/lib -DBoost_NO_SYSTEM_PATHS=ON -DBoost_NO_BOOST_CMAKE=ON )

#
# Purge runtime generated objects
purge_ko:
	rm -f ~/.local/var/bohrium/kernels/*
	rm -rf ~/.local/var/bohrium/objects/*
	rm -f ~/.local/var/bohrium/fuse_cache/*

#
# Remove installed stuff specific to the cpu-engine
purge_cpu: purge_ko
	rm -rf ~/.local/share/bohrium/templates/*
	rm -rf ~/.local/var/bohrium/scripts
	rm -f ~/.local/lib/libbh_ve_cpu.*

#
# Remove everything
purge_all: purge_cpu
	cd $(BH_SRC) && git clean -fdx && git clean -fdX
	rm -rf $(BDIR)
	rm -rf ~/.local/bin
	rm -rf ~/.local/include
	rm -rf ~/.local/lib
	rm -rf ~/.local/share/bohrium
	rm -rf ~/.local/var
	rm -f ~/.bohrium/config.ini

#
# "Install the cpu-engine

reset: purge_all clean

install: purge_ko
	mkdir -p $(BDIR)
	cd $(BDIR) && CC=$(CC) CXX=$(CXX) cmake $(BH_SRC) $(BUILD_OPTIONS) && make $(PAR_LVL) && make install

install_dev: cil_off cluster_off fun_off gpu_off install

install_debug: cil_off cluster_off fun_off gpu_off cpu_debug_on install

install_profiling: cil_off cluster_off fun_off gpu_off cpu_profiling_on install

#
# Clean up the build-files
clean:
	rm -rf $(BDIR) core trace-*.txt flow*.dot flow*.html graph-*.dot $(CPU)/doc/output

crap:
	rm -f graph*.dot flow*.dot trace*.txt flow*.html vgcore.* core /tmp/graph*.html /tmp/graph*.png /tmp/graph*.png

foo:
	BH_CPU_JIT_NESTED=1 python $(BH_SRC)/ve/cpu/tools/red.py
	BH_CPU_JIT_NESTED=1 python -m bohrium $(BH_SRC)/ve/cpu/tools/red.py

test:
	BH_CPU_JIT_LEVEL=3 python $(BH_SRC)/test/python/numpytest.py $(TEST_EXCLUDE) $(TEST)
	BH_CPU_JIT_LEVEL=1 python $(BH_SRC)/test/python/numpytest.py $(TEST_EXCLUDE) $(TEST)

#
# Generate Doxygen documentation
docs:
	mkdir -p $(CPU)/doc/output
	cd $(CPU) && doxygen doc/cpu_engine.doxy
