-- phpMyAdmin SQL Dump
-- version 3.4.10.1deb1
-- http://www.phpmyadmin.net
--
-- ホスト: localhost
-- 生成時間: 2013 年 1 月 24 日 14:40
-- サーバのバージョン: 5.5.28
-- PHP のバージョン: 5.3.10-1ubuntu3.4

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- データベース: `dcasedb`
--

-- --------------------------------------------------------

--
-- テーブルの構造 `Argument`
--

CREATE TABLE IF NOT EXISTS `Argument` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` text,
  `goal_id` int(11) DEFAULT NULL,
  `master_branch_id` int(11) DEFAULT NULL,
  `description` text,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=11 ;

--
-- テーブルのデータをダンプしています `Argument`
--

INSERT INTO `Argument` (`id`, `name`, `goal_id`, `master_branch_id`, `description`) VALUES
(1, NULL, 1, 1, NULL),
(2, NULL, 6, 2, NULL),
(3, NULL, 11, 3, NULL),
(4, NULL, 16, 4, NULL),
(5, NULL, 21, 5, NULL),
(6, NULL, 26, 6, 'Basic Syntax'),
(7, NULL, 28, 7, 'Basic Syntax'),
(8, NULL, 33, 8, 'Basic Syntax'),
(9, NULL, 38, 9, 'Basic Syntax'),
(10, NULL, 43, 10, 'Basic Syntax');

-- --------------------------------------------------------

--
-- テーブルの構造 `Branch`
--

CREATE TABLE IF NOT EXISTS `Branch` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` text,
  `Argument_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `fk_Branch_Argument1_idx` (`Argument_id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=11 ;

--
-- テーブルのデータをダンプしています `Branch`
--

INSERT INTO `Branch` (`id`, `name`, `Argument_id`) VALUES
(1, NULL, 1),
(2, NULL, 2),
(3, NULL, 3),
(4, NULL, 4),
(5, NULL, 5),
(6, NULL, 6),
(7, NULL, 7),
(8, NULL, 8),
(9, NULL, 9),
(10, NULL, 10);

-- --------------------------------------------------------

--
-- テーブルの構造 `Commit`
--

CREATE TABLE IF NOT EXISTS `Commit` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `method` text,
  `args` text,
  `argument_id` int(11) NOT NULL,
  `revision` text NOT NULL,
  `time` int(11) NOT NULL,
  `Branch_id` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `fk_Commit_Argument1` (`argument_id`),
  KEY `fk_Commit_Branch1_idx` (`Branch_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- テーブルの構造 `Context`
--

CREATE TABLE IF NOT EXISTS `Context` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `context_key` text,
  `value` text,
  `node_id` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `fk_Context_DBNode1_idx` (`node_id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=7 ;

--
-- テーブルのデータをダンプしています `Context`
--

INSERT INTO `Context` (`id`, `context_key`, `value`, `node_id`) VALUES
(1, 'OS', 'Ubuntu 12.04', 7),
(2, 'Architecture', 'x86_64', 7),
(3, 'C Compiler', 'GCC 4.6.3', 7),
(4, 'OS', 'Ubuntu 12.04', 39),
(5, 'Architecture', 'x86_64', 39),
(6, 'C Compiler', 'GCC 4.6.3', 39);

-- --------------------------------------------------------

--
-- テーブルの構造 `DBNode`
--

CREATE TABLE IF NOT EXISTS `DBNode` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` text,
  `description` text,
  `evidence_flag` tinyint(1) DEFAULT NULL,
  `nodeType_id` int(11) NOT NULL,
  `argument_id` int(11) DEFAULT NULL,
  `branch_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `fk_Node_NodeType1_idx` (`nodeType_id`),
  KEY `fk_Node_Argument1_idx` (`argument_id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=48 ;

--
-- テーブルのデータをダンプしています `DBNode`
--

INSERT INTO `DBNode` (`id`, `name`, `description`, `evidence_flag`, `nodeType_id`, `argument_id`, `branch_id`) VALUES
(1, NULL, 'Basic Syntax', 0, 1, 1, 1),
(2, NULL, 'Basic Syntax Context', 0, 4, 1, 1),
(3, NULL, 'Separated by Syntax', 0, 2, 1, 1),
(4, NULL, 'Function Decralation', 0, 1, 1, 1),
(5, NULL, 'Return Statement', 0, 1, 1, 1),
(6, NULL, 'Basic Syntax', 0, 1, 2, 2),
(7, NULL, 'Basic Syntax Context', 0, 4, 2, 2),
(8, NULL, 'Separated by Syntax', 0, 2, 2, 2),
(9, NULL, 'Function Decralation', 0, 1, 2, 2),
(10, NULL, 'Return Statement', 0, 1, 2, 2),
(11, NULL, 'Basic Syntax', 0, 1, 3, 3),
(12, NULL, 'Basic Syntax Context', 0, 4, 3, 3),
(13, NULL, 'Separated by Syntax', 0, 2, 3, 3),
(14, NULL, 'Function Decralation', 0, 1, 3, 3),
(15, NULL, 'Return Statement', 0, 1, 3, 3),
(16, NULL, 'Basic Syntax', 0, 1, 4, 4),
(17, NULL, 'Basic Syntax Context', 0, 4, 4, 4),
(18, NULL, 'Separated by Syntax', 0, 2, 4, 4),
(19, NULL, 'Function Decralation', 0, 1, 4, 4),
(20, NULL, 'Return Statement', 0, 1, 4, 4),
(21, NULL, 'Basic Syntax', 0, 1, 5, 5),
(22, NULL, 'Basic Syntax Context', 0, 4, 5, 5),
(23, NULL, 'Separated by Syntax', 0, 2, 5, 5),
(24, NULL, 'Function Decralation', 0, 1, 5, 5),
(25, NULL, 'Return Statement', 0, 1, 5, 5),
(26, NULL, 'Basic Syntax', 0, 1, 6, 6),
(27, NULL, '', 0, 4, 6, 6),
(28, NULL, 'Basic Syntax', 0, 1, 7, 7),
(29, NULL, '', 0, 4, 7, 7),
(30, NULL, 'Separated by Syntax', 0, 2, 7, 7),
(31, NULL, 'Function Decralation', 0, 1, 7, 7),
(32, NULL, 'Return Statement', 0, 1, 7, 7),
(33, NULL, 'Basic Syntax', 0, 1, 8, 8),
(34, NULL, '', 0, 4, 8, 8),
(35, NULL, 'Separated by Syntax', 0, 2, 8, 8),
(36, NULL, 'Function Decralation', 0, 1, 8, 8),
(37, NULL, 'Return Statement', 0, 1, 8, 8),
(38, NULL, 'Basic Syntax', 0, 1, 9, 9),
(39, NULL, '', 0, 4, 9, 9),
(40, NULL, 'Separated by Syntax', 0, 2, 9, 9),
(41, NULL, 'Function Decralation', 0, 1, 9, 9),
(42, NULL, 'Return Statement', 0, 1, 9, 9),
(43, NULL, 'Basic Syntax', 0, 1, 10, 10),
(44, NULL, '', 0, 4, 10, 10),
(45, NULL, 'Separated by Syntax', 0, 2, 10, 10),
(46, NULL, 'Function Decralation', 0, 1, 10, 10),
(47, NULL, 'Return Statement', 0, 1, 10, 10);

-- --------------------------------------------------------

--
-- テーブルの構造 `NodeLink`
--

CREATE TABLE IF NOT EXISTS `NodeLink` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `parent_Node_id` int(11) NOT NULL,
  `child_Node_id` int(11) NOT NULL,
  `argument_id` int(11) NOT NULL,
  `branch_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `fk_NodeLink_Node1_idx` (`parent_Node_id`),
  KEY `fk_NodeLink_Node2_idx` (`child_Node_id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=38 ;

--
-- テーブルのデータをダンプしています `NodeLink`
--

INSERT INTO `NodeLink` (`id`, `parent_Node_id`, `child_Node_id`, `argument_id`, `branch_id`) VALUES
(1, 1, 2, 1, 1),
(2, 1, 3, 1, 1),
(3, 3, 4, 1, 1),
(4, 3, 5, 1, 1),
(5, 6, 7, 2, 2),
(6, 6, 8, 2, 2),
(7, 8, 9, 2, 2),
(8, 8, 10, 2, 2),
(9, 11, 12, 3, 3),
(10, 11, 13, 3, 3),
(11, 13, 14, 3, 3),
(12, 13, 15, 3, 3),
(13, 16, 17, 4, 4),
(14, 16, 18, 4, 4),
(15, 18, 19, 4, 4),
(16, 18, 20, 4, 4),
(17, 21, 22, 5, 5),
(18, 21, 23, 5, 5),
(19, 23, 24, 5, 5),
(20, 23, 25, 5, 5),
(21, 26, 27, 6, 6),
(22, 28, 29, 7, 7),
(23, 28, 30, 7, 7),
(24, 30, 31, 7, 7),
(25, 30, 32, 7, 7),
(26, 33, 34, 8, 8),
(27, 33, 35, 8, 8),
(28, 35, 36, 8, 8),
(29, 35, 37, 8, 8),
(30, 38, 39, 9, 9),
(31, 38, 40, 9, 9),
(32, 40, 41, 9, 9),
(33, 40, 42, 9, 9),
(34, 43, 44, 10, 10),
(35, 43, 45, 10, 10),
(36, 45, 46, 10, 10),
(37, 45, 47, 10, 10);

-- --------------------------------------------------------

--
-- テーブルの構造 `NodeType`
--

CREATE TABLE IF NOT EXISTS `NodeType` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `type_name` varchar(45) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=6 ;

--
-- テーブルのデータをダンプしています `NodeType`
--

INSERT INTO `NodeType` (`id`, `type_name`) VALUES
(1, 'Goal'),
(2, 'Strategy'),
(4, 'Context'),
(5, 'Evidence');

--
-- ダンプしたテーブルの制約
--

--
-- テーブルの制約 `Branch`
--
ALTER TABLE `Branch`
  ADD CONSTRAINT `fk_Branch_Argument1` FOREIGN KEY (`Argument_id`) REFERENCES `Argument` (`id`) ON DELETE NO ACTION ON UPDATE NO ACTION;

--
-- テーブルの制約 `Commit`
--
ALTER TABLE `Commit`
  ADD CONSTRAINT `fk_Commit_Argument1` FOREIGN KEY (`argument_id`) REFERENCES `Argument` (`id`) ON DELETE NO ACTION ON UPDATE NO ACTION,
  ADD CONSTRAINT `fk_Commit_Branch1` FOREIGN KEY (`Branch_id`) REFERENCES `Branch` (`id`) ON DELETE NO ACTION ON UPDATE NO ACTION;

--
-- テーブルの制約 `Context`
--
ALTER TABLE `Context`
  ADD CONSTRAINT `fk_Context_DBNode1` FOREIGN KEY (`node_id`) REFERENCES `DBNode` (`id`) ON DELETE NO ACTION ON UPDATE NO ACTION;

--
-- テーブルの制約 `DBNode`
--
ALTER TABLE `DBNode`
  ADD CONSTRAINT `fk_Node_Argument1` FOREIGN KEY (`argument_id`) REFERENCES `Argument` (`id`) ON DELETE NO ACTION ON UPDATE NO ACTION,
  ADD CONSTRAINT `fk_Node_NodeType1` FOREIGN KEY (`nodeType_id`) REFERENCES `NodeType` (`id`) ON DELETE NO ACTION ON UPDATE NO ACTION;

--
-- テーブルの制約 `NodeLink`
--
ALTER TABLE `NodeLink`
  ADD CONSTRAINT `fk_NodeLink_Node1` FOREIGN KEY (`parent_Node_id`) REFERENCES `DBNode` (`id`) ON DELETE NO ACTION ON UPDATE NO ACTION,
  ADD CONSTRAINT `fk_NodeLink_Node2` FOREIGN KEY (`child_Node_id`) REFERENCES `DBNode` (`id`) ON DELETE NO ACTION ON UPDATE NO ACTION;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
