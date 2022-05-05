#!/bin/bash

LAST_HASH=7b6c7ea239ccb85d63cb7258d0379e92f14301ee
INPUT_FILE=$1
PKG_VERSION=$2
WORKING_DIR=$3
OUTPUT_FILE=$4

SRC_DIR="$(dirname $(realpath $0))"
pushd ${SRC_DIR}

COMMITS_ARRAY=( $(git log ${LAST_HASH}..HEAD | grep ^commit | awk {'print $2'}) )

declare -p COMMITS_ARRAY

COMMITS_DIR=${WORKING_DIR}/commits
mkdir -p ${COMMITS_DIR}
pushd ${COMMITS_DIR}

# Initialize categories
FEATURES=0
IMPROVEMENTS=0
BUGFIXES=0
MISC=0
WIP=0

echo -e "New Features:\n" > New_features.log
echo -e "Improvements:\n" > Improvements.log
echo -e "Bug Fixes:\n" > Bug_fixes.log
echo -e "Misc Changes:\n" > Misc.log
echo -e "Work In Progress:\n" > Wip.log

popd #Current dir: $SRC_DIR

# Extract tagged messages and organize into categories
for commit in ${COMMITS_ARRAY[@]}
do
  git log $commit -1 > ${COMMITS_DIR}/${commit}.log
  DATE=$(git log -1 $commit --pretty=format:%ad --date=short)

  # Features
  if sed '/Description for Open Source:/Q' ${COMMITS_DIR}/${commit}.log | grep "^[[:blank:]]*\[feature\]" ; then
    FEATURES=1
    echo "$(sed '/Description for Open Source:/Q' ${COMMITS_DIR}/${commit}.log | grep "^[[:blank:]]*\[feature\]" | sed 's/^[[:blank:]]*\[feature\][[:blank:]]*/  /') (${DATE})" >> ${COMMITS_DIR}/New_features.log
  fi

  # Improvements
  if sed '/Description for Open Source:/Q' ${COMMITS_DIR}/${commit}.log | grep "^[[:blank:]]*\[improvement\]" ; then
    IMPROVEMENTS=1
    echo "$(sed '/Description for Open Source:/Q' ${COMMITS_DIR}/${commit}.log | grep "^[[:blank:]]*\[improvement\]" | sed 's/^[[:blank:]]*\[improvement\][[:blank:]]*/  /') (${DATE})" >> ${COMMITS_DIR}/Improvements.log
  fi

  # Bug fixes
  if sed '/Description for Open Source:/Q' ${COMMITS_DIR}/${commit}.log | grep "^[[:blank:]]*\[bugfix\]" ; then
    BUGFIXES=1
    echo "$(sed '/Description for Open Source:/Q' ${COMMITS_DIR}/${commit}.log | grep "^[[:blank:]]*\[bugfix\]" | sed 's/^[[:blank:]]*\[bugfix\][[:blank:]]*/  /') (${DATE})" >> ${COMMITS_DIR}/Bug_fixes.log
  fi

  # Misc
  if sed '/Description for Open Source:/Q' ${COMMITS_DIR}/${commit}.log | grep "^[[:blank:]]*\[misc\]" ; then
    MISC=1
    echo "$(sed '/Description for Open Source:/Q' ${COMMITS_DIR}/${commit}.log | grep "^[[:blank:]]*\[misc\]" | sed 's/^[[:blank:]]*\[misc\][[:blank:]]*/  /') (${DATE})" >> ${COMMITS_DIR}/Misc.log
  fi

  # Work In Progress
  if sed '/Description for Open Source:/Q' ${COMMITS_DIR}/${commit}.log | grep "^[[:blank:]]*\[wip\]" ; then
    WIP=1
    echo "$(sed '/Description for Open Source:/Q' ${COMMITS_DIR}/${commit}.log | grep "^[[:blank:]]*\[wip\]" | sed 's/^[[:blank:]]*\[wip\][[:blank:]]*/  /') (${DATE})" >> ${COMMITS_DIR}/Wip.log
  fi

  # No-Changelog
done

# Changelog creation
pushd ${COMMITS_DIR}
echo -e " C for Metal (CM) CPU Emulation changes included in $PKG_VERSION release\n" > CHANGELOG.txt
if [[ $FEATURES -eq 1 ]] ; then
  cat New_features.log | sed -r '/\[no-changelog|wip\]/d' >> CHANGELOG.txt
  echo >> CHANGELOG.txt
fi
if [[ $IMPROVEMENTS -eq 1 ]] ; then
  cat Improvements.log | sed -r '/\[no-changelog|wip\]/d' >> CHANGELOG.txt
  echo >> CHANGELOG.txt
fi
if [[ $BUGFIXES -eq 1 ]] ; then
  cat Bug_fixes.log | sed -r '/\[no-changelog|wip\]/d' >> CHANGELOG.txt
  echo >> CHANGELOG.txt
fi
if [[ $MISC -eq 1 ]] ; then
  cat Misc.log | sed -r '/\[no-changelog|wip\]/d' >> CHANGELOG.txt
  echo >> CHANGELOG.txt
fi
if [[ $WIP -eq 1 ]] ; then
  cat Wip.log | sed '/\[no-changelog\]/d' >> CHANGELOG.txt
  echo >> CHANGELOG.txt
fi
echo >> CHANGELOG.txt
cat ${INPUT_FILE} >> CHANGELOG.txt

cp CHANGELOG.txt ${OUTPUT_FILE}

popd
popd
