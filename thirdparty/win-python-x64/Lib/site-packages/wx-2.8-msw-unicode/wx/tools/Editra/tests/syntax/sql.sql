/******************************************
* Syntax Highlighting Test File for SQL   *
* Multi-Line Comment Block                *
* Oracle 9i SQL                           *
*******************************************/
--- Single Line Comments are like this

--- Drop all tables, in case they were previously created ---
DROP TABLE shipment;
DROP TABLE customer;
DROP TABLE truck;
DROP TABLE city;

--- Create the customer table ---
CREATE TABLE customer
(
	CUS_ID	     Char(4) CONSTRAINT cus_id_pk PRIMARY KEY,
	CUS_LNAME    Varchar2(20),
	CUS_FNAME    Varchar2(20),
	ANN_REVENUE  Number(12,2),
	CUS_TYPE     Char(1)
);

--- Create the truck table ---
CREATE TABLE truck
(
	TRUCK_ID     Char(4)	   CONSTRAINT truck_id_pk PRIMARY KEY,
	DRIVER_NAME  Varchar2(40)
);

--- Create the city table ---
CREATE TABLE city
(
	CITY_ID	     Varchar2(4)   CONSTRAINT city_id_pk PRIMARY KEY,
	CITY_NAME    Varchar2(30),
	CITY_STATE   Char(2),
	POPULATION   Number(10)
);

--- Create the shipment table ---
CREATE TABLE shipment
(
   SHIPMENT_ID Char(4)	   CONSTRAINT ship_id_pk PRIMARY KEY,
   CUS_ID      Char(4)	   CONSTRAINT cust_id_fk REFERENCES customer(cus_id),
   WEIGHT      Number(12,2),
   TRUCK_ID    Char(4)	   CONSTRAINT truck_id_fk REFERENCES truck(truck_id),
   CITY_ID     Varchar2(4) CONSTRAINT city_id_fk  REFERENCES city(city_id),
   SHIP_DATE   DATE
);

--- Insert records into customer table ---
INSERT INTO customer VALUES
	('C101','Smith','Joe',3000000.3,'P');
INSERT INTO customer VALUES
	('C102','Sneider','Jenny',7000000.5,'P');
INSERT INTO customer VALUES
	('C103','Robinson','Dan',1000000.8,'C');
COMMIT;

--- Insert records into truck table ---
INSERT INTO truck VALUES
	('T101','Dan Brun');
INSERT INTO truck VALUES
	('T102','Bob Lee');
INSERT INTO truck VALUES
	('T104','Jerry Carlson');
INSERT INTO truck VALUES
	('T103','Frank Hong');
COMMIT;

--- Insert records into city table ---
INSERT INTO city VALUES
	('101','Dekalb','IL',50000);
INSERT INTO city VALUES
	('201','Lincoln','NE',160000);
INSERT INTO city VALUES
	('301','Houston','TX',800000);
INSERT INTO city VALUES
	('401','Laredo','TX',260000);
COMMIT;

--- Insert records into shipment table ---
INSERT INTO shipment VALUES
	('2001','C101',2500.2,'T101','101','12-Apr-2002');
INSERT INTO shipment VALUES
	('2002','C102',7500.7,'T101','201','20-Apr-2002');
INSERT INTO shipment VALUES
	('2003','C103',800000.8,'T103','201','25-May-2002');
INSERT INTO shipment VALUES
	('2004','C102',95.00,'T102','301','02-May-2003');
INSERT INTO shipment VALUES
	('2005','C101',85.00,'T102','401','02-May-2003');
COMMIT;

--- Queries 1 - 10 ---

--- How many shipments between 1/1/02 & 5/1/03?
--- Version 1 shows all records between the given dates
SELECT *
FROM   shipment
WHERE  SHIP_DATE >= '01-Jan-2002' 
AND    SHIP_DATE <= '01-May-2003';

--- Version 2 returns simply a count of all the given dates
SELECT COUNT(*)
FROM   shipment
WHERE  SHIP_DATE >= '01-Jan-2002' 
AND    SHIP_DATE <= '01-May-2003';

--- What is destination city name of shipment id# 2004
SELECT CITY_NAME
FROM   shipment,city
WHERE  SHIPMENT_ID = '2004' 
AND    shipment.CITY_ID = city.CITY_ID;

--- What are the truck ids of trucks that have carried 
--- shipments over 100 lbs?
SELECT DISTINCT TRUCK_ID
FROM   shipment
WHERE  WEIGHT >= 100;

--- Give the Names of customers who have sent shipments to cities 
--- starting with 'L'?
SELECT CUS_LNAME, CUS_FNAME
FROM   customer,shipment,city
WHERE  customer.CUS_ID = shipment.CUS_ID 
AND    shipment.CITY_ID = city.CITY_ID 
AND    city.CITY_NAME LIKE 'L%';

--- What are the names of customers who have sent packages to 
--- Lincoln, NE?
SELECT CUS_LNAME, CUS_FNAME
FROM   customer,shipment,city
WHERE  customer.CUS_ID = shipment.CUS_ID 
AND    shipment.CITY_ID = city.CITY_ID 
AND    city.CITY_NAME = 'Lincoln';

--- Who are the customers having over 5 million in revenue and 
--- have sent less than 100lbs?
SELECT DISTINCT CUS_FNAME, CUS_LNAME
FROM   customer, shipment
WHERE  customer.ANN_REVENUE > 5000000 
AND    shipment.WEIGHT < 100;

--- For each customer what is the average weight of a package, 
--- show name and avg weight?
SELECT   CUS_FNAME, CUS_LNAME, AVG(WEIGHT)
FROM     customer,shipment
WHERE    customer.CUS_ID = shipment.CUS_ID
GROUP BY CUS_FNAME, CUS_LNAME;

--- For each city with a population over 100,000 what is the 
--- minimum weight of a package sent there?
SELECT   CITY_NAME, MIN(WEIGHT)
FROM     city,shipment
WHERE    city.POPULATION >= 100000
AND      city.CITY_ID = shipment.CITY_ID
GROUP BY CITY_NAME;

--- For each city that has recieved at least 2 packages, what is the 
--- average weight of a package sent to that city?
SELECT   CITY_NAME, COUNT(SHIPMENT_ID), AVG(WEIGHT)
FROM     city,shipment
WHERE    shipment.CITY_ID = city.CITY_ID
GROUP BY CITY_NAME
HAVING   COUNT(shipment.CITY_ID) >= 2;
