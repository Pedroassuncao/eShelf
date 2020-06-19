
USE master; 

IF EXISTS(SELECT * FROM sys.databases WHERE NAME='eshelf') DROP DATABASE eshelf;




create database eshelf



USE eshelf;

create table product(
ID int not null primary key,
brand char (25),
model char (25) unique,
Baseprice float 
);

create table promotions(
	model char (25) not null primary key,
	startDate date,
	endDate date, 
	discount int,
	priceProm int,
	FOREIGN KEY (model) REFERENCES product(model),
	check (endDate > startDate)
);

create table stock(
	product_ID int primary key REFERENCES product(ID),
	quantidade int
);



create table animations(
	ID_anim int primary key references product(ID),
	anim varchar (255)
);