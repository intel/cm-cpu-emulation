#!/bin/bash

# args
LAST_HASH=24f1344ec863d58f505280547fc17710a42929dd
INPUT_FILE=$1
PKG_VERSION=$2
WORKING_DIR=$3
OUTPUT_FILE=$4

# get current commits into array
SRC_DIR="$(dirname $(realpath $0))"
pushd ${SRC_DIR}
COMMITS_ARRAY=( $(git log ${LAST_HASH}..HEAD | grep ^commit | awk {'print $2'}) )
declare -p COMMITS_ARRAY

COMMITS_DIR=${WORKING_DIR}/commits
mkdir -p ${COMMITS_DIR}
pushd ${COMMITS_DIR}

# Initialize categories
feature=0
improvement=0
bugfix=0
misc=0
wip=0

echo -e "New Features:\n" > feature.log
echo -e "Improvements:\n" > improvement.log
echo -e "Bug Fixes:\n" > bugfix.log
echo -e "Misc Changes:\n" > misc.log
echo -e "Work In Progress:\n" > wip.log

popd #Current dir: $SRC_DIR

# Extract tagged messages and organize into categories
for commit in ${COMMITS_ARRAY[@]}
do
  git log $commit -1 > ${COMMITS_DIR}/${commit}.log
  DATE=$(git log -1 $commit --pretty=format:%ad --date=short)

  #New code to be tested
  for category in feature improvement bugfix misc wip
  do
    if sed '/OPEN SOURCE BEGIN/Q' ${COMMITS_DIR}/${commit}.log | grep "^[[:blank:]]*\[$category\]" ; then
      # set $category to 1 means that category should go in the changelog
      eval declare \$category=1
      echo "$(sed '/OPEN SOURCE BEGIN/Q' ${COMMITS_DIR}/${commit}.log | grep "^[[:blank:]]*\[$category\]" | sed "s/^[[:blank:]]*\[$category\][[:blank:]]*/  /") (${DATE})" >> ${COMMITS_DIR}/$category.log
    fi
  done
done

# Changelog creation
CHANGELOG=CHANGELOG.txt
pushd ${COMMITS_DIR}
echo -e " C for Metal (CM) CPU Emulation changes included in $PKG_VERSION release\n" > $CHANGELOG
for category in feature improvement bugfix misc
do
  if [[ $(eval echo "\$$category") -eq 1 ]] ; then
    cat $category.log | sed -r '/\[no-changelog|wip\]/d' >> $CHANGELOG
    echo >> $CHANGELOG
  fi
done
if [[ $wip -eq 1 ]] ; then
  cat wip.log | sed '/\[no-changelog\]/d' >> $CHANGELOG
  echo >> $CHANGELOG
fi
echo >> $CHANGELOG
cat ${INPUT_FILE} >> $CHANGELOG

cp $CHANGELOG ${OUTPUT_FILE}

popd
popd
