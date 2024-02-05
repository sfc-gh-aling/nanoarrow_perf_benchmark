# dateset

## arrow_perf_int

2 batches in the data, each batch has ~102k rows, the schema of the data contains one integer column.

the dateset contains random int data like the following:

```python
(-8921048594857845868,)
(4525876191872365692,)
(-2032490112803869515,)
(-3022312598505478628,)
(-7432561891101019917,)
(5889981025522118186,)
(-235220727786376659,)
(5179122178562164530,)
```

## arrow_perf_str

2 batches in the data, each batch has ~52k rows, the schema of the data contains one string column.

the dateset contains random string data like the following:

```python
('WDYpU9br8HhgEz8OrePJ',)
('LHKLkMgLiSyX6a5KdXFC',)
('bjK39vEaFRAp9tXR85YG',)
('teGKGHAIVbr9RBlXtVbM',)
('H6QmSPs9eI6zpdspMxmi',)
('tKGxrXPYAa0Mbh89YNXW',)
('5G0helUf0DUGExaIHUN9',)
('TnjKnhKyKH1c1ObS4Lqz',)
('aDVLjnmzUFDP23FtfHRj',)
('z3RW7h6VO4VrvleKNYDz',)
```

## arrow_perf_float

3 batches in the data, 1-2nd batches contain ~130k rows per batch, 3rd batch contains ~75k rows,
the schema of the data contains one float column.

the dateset contains random string data like the following:

```python
(1.508774,)
(0.44382,)
(-1.006902,)
(0.6014,)
(1.066818,)
(-0.88114,)
(-8.532491,)
(-0.945349,)
(-0.105462,)
```

## arrow_perf_decimal

10 batches in the data, 1-9th batches contains ~53k rows perf batch, 10th batch contains ~25k rows,
the schema of the data contains one decimal column.

the dateset contains random string data like the following:

```python
(Decimal('1.0000000000'),)
(Decimal('3.0000000000'),)
(Decimal('4.0000000000'),)
(Decimal('0E-10'),)
(Decimal('1.0000000000'),)
(Decimal('0E-10'),)
(Decimal('1.0000000000'),)
(Decimal('-2.0000000000'),)
```

## references

SQL used to generate test data:

```sql
create table arrow_perf_table_int(col int);
insert into arrow_perf_table_int SELECT random() FROM table(generator(rowCount => 1000000));
select * from arrow_perf_table_int;

create table arrow_perf_table_str(col string);
insert into arrow_perf_table_str SELECT randstr(20, random()) FROM table(generator(rowCount => 1000000));
select * from arrow_perf_table_str;

create table arrow_perf_table_float(col float);
insert into arrow_perf_table_float SELECT random()/random() FROM table(generator(rowCount => 1000000));
select * from arrow_perf_table_float;

create table arrow_perf_table_decimal(col DECIMAL(20,10));
insert into arrow_perf_table_decimal SELECT TO_DECIMAL(random()/random()) FROM table(generator(rowCount => 1000000));
select * from arrow_perf_table_decimal;
```
