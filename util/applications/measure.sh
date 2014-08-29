#!/bin/bash

function feeder()
{
    local num=$1
    local top_opts="-b"
    [[ -n "num" ]] && top_opts="$top_opts -n $num"
    # make all buffering per-line
    stdbuf -oL -eL top $top_opts | grep --line-buffered -A 4 ^top
}

function extract_cpus()
{
    local timestamp=$1
    local input=$2
    local us=${input%% us, *}
    us=${us##%Cpu(s): }
    local sy=${input%% sy, *}
    sy=${sy##* us, }
    local total=$( echo "$us + $sy" | bc )
    echo "$timestamp,$us,$sy,$total"
}

function extract_mem()
{
    local timestamp=$1
    local input=$2
    local used=${input%% used, *}
    used=${used##* total, *( )}
    echo "$timestamp,$used"
}

function extract_swap()
{
    local timestamp=$1
    local input=$2
    local used=${input%% used, *}
    used=${used##* total, *( )}
    echo "$timestamp,$used"
}

function parser()
{
    shopt -s extglob
    local cpus_out=$1
    local mem_out=$2
    local swap_out=$3
    local timestamp
    while true ; do
        read line
        [[ -z "$line" ]] && break
        timestamp=$( date +%Y%m%dT%H%M%S )
        [[ "$line" = "%Cpu(s):"* ]]  && extract_cpus "$timestamp" "$line" >> "$cpus_out"
        [[ "$line" = "KiB Mem:"* ]]  && extract_mem  "$timestamp" "$line" >> "$mem_out"
        [[ "$line" = "KiB Swap:"* ]] && extract_swap "$timestamp" "$line" >> "$swap_out"
    done
}

>cpus.log
>mem.log
>swap.log
feeder 1000 | parser cpus.log mem.log swap.log
