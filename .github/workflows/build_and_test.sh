#!/usr/bin/env bash
set -euo pipefail

# initialize PR comment file if running in a pull request
if [ "${GITHUB_EVENT_NAME:-}" = "pull_request" ]; then
    echo "**Documentation Previews:**" > pr_comment.md
fi

# 1. identify changed files
CHANGED_FILES=()
if [ "${GITHUB_EVENT_NAME:-}" = "pull_request" ]; then
    # fetch base ref to make sure it's available for diffing
    git fetch origin "${GITHUB_BASE_REF}" --depth=1 || true
    base_ref="origin/${GITHUB_BASE_REF}"

    echo "Comparing against base branch: ${base_ref}"

    mapfile -t CHANGED_FILES < <(git diff --name-only "${base_ref}...HEAD" || git diff --name-only "${base_ref}" HEAD || true)
elif [ -n "${GITHUB_SHA:-}" ]; then
    # for pushes, compare against the previous commit
    before_sha="${GITHUB_EVENT_BEFORE:-}"

    if [ -z "$before_sha" ] || [ "$before_sha" = "0000000000000000000000000000000000000000" ]; then
        echo "New branch/tag push or no before SHA, comparing against parent commit HEAD~1"

        mapfile -t CHANGED_FILES < <(git diff --name-only HEAD~1 HEAD || true)
    else
        echo "Comparing ${before_sha} to ${GITHUB_SHA}"

        mapfile -t CHANGED_FILES < <(git diff --name-only "${before_sha}" "${GITHUB_SHA}" || true)
    fi
else
    # local run or fallback
    echo "Local run, comparing against HEAD~1"

    mapfile -t CHANGED_FILES < <(git diff --name-only HEAD~1 HEAD || true)
fi

echo "Changed files:"
for file in "${CHANGED_FILES[@]}"; do
    echo "  $file"
done

# determine if we should build everything
BUILD_ALL=false
if [ ${#CHANGED_FILES[@]} -eq 0 ]; then
    echo "No changed files detected. Fallback: building all subsystems."

    BUILD_ALL=true
else
    for file in "${CHANGED_FILES[@]}"; do
        # if workflow itself, docs, or top-level files changed, build everything
        if [[ "$file" =~ ^\.github/ ]] || [[ ! "$file" =~ / ]]; then
            echo "Global file or workflow changed: $file. Building all subsystems."

            BUILD_ALL=true

            break
        fi
    done
fi

# 2. find all firmware directories containing a CMakeLists.txt
firmware_dirs=$(find . -maxdepth 4 -name "CMakeLists.txt" | grep "/firmware/CMakeLists.txt" | sed 's|/CMakeLists.txt||' | sort -u)

# create documentation output directory
mkdir -p docs_out

for fw_dir in $firmware_dirs; do
    # remove leading ./
    fw_dir="${fw_dir#./}"

    # get the parent subsystem directory (e.g. eps/v1 or obc/v1)
    subsystem_dir=$(dirname "$fw_dir")

    # check if this subsystem is affected
    is_affected=false
    if [ "$BUILD_ALL" = true ]; then
        is_affected=true
    else
        for file in "${CHANGED_FILES[@]}"; do
            if [[ "$file" == "$subsystem_dir"* ]]; then
                is_affected=true

                echo "Subsystem ${subsystem_dir} is affected by change in ${file}"

                break
            fi
        done
    fi

    if [ "$is_affected" = false ]; then
        echo "Skipping unaffected subsystem: ${subsystem_dir}"
        continue
    fi

    echo "::group::Building and testing ${subsystem_dir}"
    echo "Processing subsystem: ${subsystem_dir} in directory: ${fw_dir}"

    # install Python requirements if they exist
    req_file=""
    if [ -f "${subsystem_dir}/documentation/sphinx/requirements.txt" ]; then
        req_file="${subsystem_dir}/documentation/sphinx/requirements.txt"
    elif [ -f "${fw_dir}/documentation/sphinx/requirements.txt" ]; then
        req_file="${fw_dir}/documentation/sphinx/requirements.txt"
    fi

    if [ -n "$req_file" ]; then
        echo "Installing Python dependencies from $req_file"

        if pip3 install --help | grep -q "break-system-packages"; then
            pip3 install --break-system-packages -r "$req_file"
        else
            pip3 install -r "$req_file"
        fi
    fi

    # configure CMake for ARM
    toolchain_args=""
    if [ -f "${fw_dir}/arm-none-eabi-toolchain.cmake" ]; then
        toolchain_args="-DCMAKE_TOOLCHAIN_FILE=${PWD}/${fw_dir}/arm-none-eabi-toolchain.cmake"
    fi

    echo "Configuring CMake for ARM..."
    cmake -B "${fw_dir}/build_arm" -S "${fw_dir}" $toolchain_args

    echo "Building ARM firmware..."
    cmake --build "${fw_dir}/build_arm"

    # run host/HITL tests if they exist
    if [ -f "${fw_dir}/tests/CMakeLists.txt" ]; then
        echo "Tests found. Configuring CMake for Host/HITL..."
        cmake -B "${fw_dir}/build_hitl" -S "${fw_dir}" -DTARGET_ARCH=HOST -DBUILD_HITL=ON

        echo "Building Host/HITL Tests..."
        cmake --build "${fw_dir}/build_hitl"

        echo "Running CTest..."
        (cd "${fw_dir}/build_hitl" && ctest --output-on-failure)
    else
        echo "No unit tests found for ${subsystem_dir}."
    fi

    # build documentation if target exists
    if grep -q "add_custom_target.*docs" "${fw_dir}/CMakeLists.txt"; then
        echo "Generating documentation..."

        cmake --build "${fw_dir}/build_arm" --target docs

        # copy Doxygen output if it exists
        doxy_dir=""
        if [ -d "${subsystem_dir}/documentation/doxygen_output" ]; then
            doxy_dir="${subsystem_dir}/documentation/doxygen_output"
        elif [ -d "${fw_dir}/build_arm/documentation/doxygen" ]; then
            doxy_dir="${fw_dir}/build_arm/documentation/doxygen"
        fi

        if [ -n "$doxy_dir" ] && [ "$(ls -A "$doxy_dir" 2>/dev/null)" ]; then
            mkdir -p "docs_out/${subsystem_dir}/doxygen"
            cp -R "$doxy_dir"/* "docs_out/${subsystem_dir}/doxygen/"
            echo "Copied Doxygen docs from $doxy_dir to docs_out/${subsystem_dir}/doxygen/"
        fi

        # copy Sphinx output if it exists
        sphinx_dir=""
        if [ -d "${fw_dir}/build_arm/documentation/sphinx" ]; then
            sphinx_dir="${fw_dir}/build_arm/documentation/sphinx"
        fi

        if [ -n "$sphinx_dir" ] && [ "$(ls -A "$sphinx_dir" 2>/dev/null)" ]; then
            mkdir -p "docs_out/${subsystem_dir}/sphinx"
            cp -R "$sphinx_dir"/* "docs_out/${subsystem_dir}/sphinx/"
            echo "Copied Sphinx docs from $sphinx_dir to docs_out/${subsystem_dir}/sphinx/"

            # generate preview links for PR comments
            if [ "${GITHUB_EVENT_NAME:-}" = "pull_request" ]; then
                owner_repo="${GITHUB_REPOSITORY}"
                owner="${owner_repo%/*}"
                repo="${owner_repo#*/}"
                pr_num="${GITHUB_EVENT_NUMBER:-}"

                url="https://${owner}.github.io/${repo}/pr-${pr_num}/${subsystem_dir}/sphinx/index.html"
                echo "- **${subsystem_dir^^} Sphinx Documentation:** [View Here]($url)" >> pr_comment.md

                if [ -d "docs_out/${subsystem_dir}/doxygen" ]; then
                    url_doxy="https://${owner}.github.io/${repo}/pr-${pr_num}/${subsystem_dir}/doxygen/index.html"
                    echo "- **${subsystem_dir^^} Doxygen API Docs:** [View Here]($url_doxy)" >> pr_comment.md
                fi
            fi
        fi
    else
        echo "No documentation target found for ${subsystem_dir}."
    fi

    echo "::endgroup::"
done
