# csv

## accumulate values

### take velocities, calculate distance

generate sample file with velocity a m/sec at each given time:

```bash
cat <<eof > velocities.csv
20200101T000000,0.7
20200101T000001,1.1
20200101T000002,1.1
20200101T000003,0.9
20200101T000004,1.3
eof 

```

append distance travelled to each data point:

```bash
cat velocities.csv \
    | csv-shuffle --fields t,v --output-fields t,t,v \
    | csv-time --to seconds --fields ,t \
    | csv-eval --init-values "prev=0;sum=0" --fields ,cur,v "sum+=(cur-prev)*(prev>0)*v;prev=cur" \
    | csv-shuffle --fields t,,v,d --output-fields t,v,d
```
