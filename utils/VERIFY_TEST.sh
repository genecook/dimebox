#!/bin/sh -f

if [ -z $DIMEBOX_HOME ]; then
  export DIMEBOX_HOME=$HOME/Desktop/dimebox;
fi

# remove files from previous run...

rm -f $1.reference_sim_log $1.reference_trace $1.reference_signature \
      $1.test_sim_log $1.test_trace $1.test_signature


# generate the reference simulation trace...
# ---------------------------------------------
$DIMEBOX_HOME/utils/RUN_OVPsim.sh $1 >$1.reference_sim_log 2>&1

if [ $? -ne 0 ]
then
    echo "FAIL???: Problems simulating test on 'golden model'."
    exit 1
fi

cat $1.reference_sim_log | $DIMEBOX_HOME/utils/reformat_ovp_trace.pl >$1.reference_trace

if [ $? -ne 0 ]
then
    echo "FAIL???: Unable to reformat 'golden model' simulation trace."
    exit 1
fi

if [ -f $1.signature ]
then
    mv $1.signature $1.reference_signature
else
    touch $1.reference_signature
fi


# generate simulation trace from testcase using dimebox...
# --------------------------------------------------------
$DIMEBOX_HOME/utils/RUN_dimebox.sh $1 >$1.test_sim_log 2>&1

if [ $? -ne 0 ]
then
    echo "FAIL???: test fails to simulate correctly on dimebox."
    exit 1
fi

cat $1.test_sim_log | $DIMEBOX_HOME/utils/reformat_dimebox_trace.pl >$1.test_trace

if [ $? -ne 0 ]
then
    echo "FAIL???: Unable to reformat dimebox simulation trace."
    exit 1
fi

if [ -f test.signature ]
then
    mv test.signature ${1}.test_signature
else
    touch ${1}.test_signature
fi


# diff the reference sim trace against dimebox sim trace...
# ---------------------------------------------------------
/usr/bin/diff $1.reference_trace $1.test_trace

if [ $? -ne 0 ]
then
    echo "FAIL???: dimebox simulation trace does NOT match reference simulation trace."
    exit 1
fi

# diff the reference test signature against dimebox test signature...
# -------------------------------------------------------------------
/usr/bin/diff $1.reference_signature $1.test_signature

if [ $? -ne 0 ]
then
    echo "FAIL???: dimebox simulation trace does NOT match reference simulation trace."
    exit 1
fi

# resimulate using dimebox generated test-image...
# --------------------------------------------------------
$DIMEBOX_HOME/utils/RESIM_dimebox.sh $1 >$1.test_resimulate_log 2>&1

if [ $? -ne 0 ]
then
    echo "FAIL???: dimebox-generated test-image fails to resimulate correctly on dimebox."
    exit 1
fi

cat $1.test_resimulate_log | $DIMEBOX_HOME/utils/reformat_dimebox_trace.pl >$1.test_resimulate_trace

if [ $? -ne 0 ]
then
    echo "FAIL???: Unable to reformat dimebox resimulation trace."
    exit 1
fi

# diff the reference sim trace against dimebox sim trace...
# ---------------------------------------------------------
/usr/bin/diff $1.test_trace $1.test_resimulate_trace

if [ $? -ne 0 ]
then
    echo "FAIL???: dimebox resimulation trace does NOT match dimebox test trace."
    exit 1
fi

echo "SUCCESS!"








  
