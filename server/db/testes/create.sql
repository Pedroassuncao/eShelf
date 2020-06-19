drop database if exists `eshelf`;
create database `eshelf` default character set utf8;
use `eshelf`;

create table `products`(
  `id`          varchar(45) not null,
  `price`       varchar(45),
  `currency`    varchar(3),
  `item`        varchar(45),
  `brand`       varchar(45),
  `model`       varchar(45),
  `description` varchar(45),
  primary key (`id`)
);
