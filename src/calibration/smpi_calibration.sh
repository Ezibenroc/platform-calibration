extract_keyword () {
    filename=$1
    keyword=$2
    match=$(cat /tmp/paravance.xml| tr ' ' '\n' | grep -o "$keyword=\".*\"")
    match=$(echo $match | sed "s/$keyword=//g" | sed "s/\"//g")
    echo $match
}

if [ $# -ne 2 ] ; then
    echo "Syntax: $0 <platform.xml> <expfile.csv>"
    exit 1
fi
platform=$1
expfile=$2

echo "Repository hash: $(git rev-parse HEAD)"
echo "Simgrid hash:    $(smpirun --git-version)"

cat <<< '
diff --git a/src/calibration/Makefile b/src/calibration/Makefile
index 0dddb38..a83ec49 100644
--- a/src/calibration/Makefile
+++ b/src/calibration/Makefile
@@ -1,4 +1,4 @@
-CC=mpicc
+CC=smpicc
 CFLAGS=-I$(IDIR)
 
 LDIR=
diff --git a/src/calibration/calibrate.c b/src/calibration/calibrate.c
index 343db22..0517e4d 100644
--- a/src/calibration/calibrate.c
+++ b/src/calibration/calibrate.c
@@ -39,7 +39,7 @@ We can set the maximum limit for the sizes with -s (as sizes go up to 1GB in zoo
 #include "utils.h"
 
 #define MAX_LINES 2000
-#define NB_RUNS 10
+#define NB_RUNS 1
 #define MAX_NAME_SIZE 256
 
 static void * my_send_buffer;
@@ -149,6 +149,7 @@ void test_op(FILE *output_files[], int op_id, int size, int nb_runs, unsigned lo
 
 int main(int argc, char** argv){
 
+  MPI_Init(&argc, &argv);
   bzero (&arguments, sizeof(my_args));
 
 
@@ -173,7 +174,6 @@ int main(int argc, char** argv){
   int size;
   int m=0;
 
-  MPI_Init(&argc, &argv);
 
   FILE *output_files[6];
   for(int i = 0; names[i]; i++) {
' > /tmp/makefile_diff.patch

git apply /tmp/makefile_diff.patch
make calibrate

prefix=$(extract_keyword $platform prefix)
suffix=$(extract_keyword $platform suffix)
echo "${prefix}1${suffix}" > /tmp/hosts.txt
echo "${prefix}2${suffix}" >> /tmp/hosts.txt

echo "Running a calibration with hosts $(cat /tmp/hosts.txt | tr '\n' ' ')"
echo "Using platform file:"
cat $platform
echo

echo "Experiment file: $(wc -l $expfile | cut -f1 -d' ') lines, hash: $(sha256sum $expfile | cut -f1 -d' ')"

echo

rm -rf /tmp/exp /tmp/exp.zip
mkdir /tmp/exp
smpirun -wrapper /usr/bin/time --cfg=smpi/privatize-global-variables:dlopen --cfg=smpi/display-timing:yes -platform \
    $platform -hostfile /tmp/hosts.txt -np 2 ./calibrate -d /tmp/exp -m 1 -M 1000000 -p exp -s $expfile

echo

pushd /tmp
zip -r exp.zip exp
popd

git reset --hard master && git clean -xfd
echo
echo "Experiment archive created: /tmp/exp.zip"
