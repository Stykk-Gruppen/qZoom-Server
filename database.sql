DROP TABLE IF EXISTS user;
DROP TABLE IF EXISTS room;
DROP TABLE IF EXISTS roomSession;

CREATE TABLE IF NOT EXISTS user
(
	id INT AUTO_INCREMENT PRIMARY KEY,
	streamId VARCHAR(32) NOT NULL UNIQUE,
	username VARCHAR(255) NOT NULL UNIQUE,
	password VARCHAR(255) NOT NULL,
	timeCreated DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS room
(
	id VARCHAR(32) NOT NULL PRIMARY KEY,
	host INT UNIQUE NOT NULL,
	password VARCHAR(255)
);

CREATE TABLE IF NOT EXISTS roomSession
(
	roomId VARCHAR(32) NOT NULL,
	userId INT NOT NULL UNIQUE,
	ipAddress VARCHAR(32) NOT NULL
);

INSERT INTO user (streamId, username, password)
VALUES ("Alpha", "stian", "123"),
("Bravo", "tarald", "123"),
("Charlie", "kent", "123");

INSERT INTO room (id, host, password)
VALUES ("Delta", 1, "123");