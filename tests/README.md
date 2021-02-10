# Testing suite

The default testing parameters are located in `common.sh`, designed for stress testing, and are too heavy for sanity testing.
Reduce the workload to suite your needs, recommended at:

```
P_MAXSIZE=5 P_CONCUR=10 make check
```
