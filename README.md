There are 3 groups of scripts:

* `channel_distrs_full_loop.py` which calls `stage2.py` does the correct and full selection of events in the skimmed ntuples, with all systematics
* `loop_selections.py` produces the full set of plots from the output of `stage2`
* and a short-cut script `sumup_ttree_draw.py` to run directly on ntuples or stage2 output.

Examples of how to run this stuff are in the log file `last_procs`.

