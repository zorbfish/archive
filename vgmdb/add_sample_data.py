#!/usr/local/bin/python

"""Build script

Rebuilds the vgmdb_cddb table for its new structure and
adds a few data samples into it. This is throwaway code for testing
"""

import MySQLdb

GET_EVERY_CD_ALBUM = "SELECT `albumid`, `parentid`, `discs` FROM \
vgmdb_albums WHERE `media` LIKE 'cd'"
GET_TRACKTIMES = 'SELECT `length` FROM `vgmdb_tracks` \
INNER JOIN `vgmdb_tracklists` ON `vgmdb_tracklists`.`tracklistid` = \
`vgmdb_tracks`.`tracklistid` WHERE `vgmdb_tracklists`.`albumid` = %s \
AND `vgmdb_tracklists`.`main` = 1 AND `vgmdb_tracks`.`disc` = %s ORDER BY \
disc, track ASC'

def add_sample(c, data):
    c.execute('SELECT `CDid` FROM `vgmdb_CDs` WHERE `albumid` = %s AND \
`disc`=%s', (data[1], data[2]))
    CDid, = c.fetchone()
    c.execute('INSERT INTO `vgmdb_discids` (`CDid`, `discid`) VALUES (%s, %s)',
              (CDid, data[0]))

if __name__ == '__main__':
    db = MySQLdb.connect(db='vgmdb', user='root', host='localhost')

    cddb = db.cursor()
    albums = db.cursor()
    track_lengths = db.cursor()

    cddb.execute('DROP TABLE IF EXISTS `vgmdb_cddb`')
    
    cddb.execute('DROP TABLE IF EXISTS `vgmdb_CDs`')
    cddb.execute("CREATE TABLE `vgmdb_CDs` (\
`CDid` int(10) unsigned NOT NULL AUTO_INCREMENT, \
`albumid` int(10) unsigned NOT NULL, \
`disc` tinyint(3) unsigned NOT NULL DEFAULT '1', \
`length` int(10) NOT NULL, \
`tracks` int(10) NOT NULL, \
PRIMARY KEY (`CDid`) \
) ENGINE=MyISAM  DEFAULT CHARSET=ascii ROW_FORMAT=DYNAMIC AUTO_INCREMENT=1")

    cddb.execute('DROP TABLE IF EXISTS `vgmdb_discids`')
    cddb.execute("CREATE TABLE `vgmdb`.`vgmdb_discids` (\
`id` INT( 10 ) UNSIGNED NOT NULL AUTO_INCREMENT ,\
`CDid` INT( 10 ) UNSIGNED NOT NULL ,\
`discid` VARCHAR( 8 ) NOT NULL ,\
`reads` INT( 10 ) NOT NULL DEFAULT '1',\
`last_read` TIMESTAMP ON UPDATE CURRENT_TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ,\
PRIMARY KEY ( `id` ) \
) ENGINE=MyISAM  DEFAULT CHARSET=ascii ROW_FORMAT=DYNAMIC AUTO_INCREMENT=1")
    
    albums.execute(GET_EVERY_CD_ALBUM)
    for (albumid, parentid, total_discs) in albums:
        for disc in range(1, total_discs + 1):
            found_tracks = track_lengths.execute(GET_TRACKTIMES,
                                                 (parentid or albumid, disc))
            if not found_tracks:
                continue
            
            # Add 2 second leadin
            seconds = [2L]

            # Record returned as (length, ) for some
            # stupid reason so a list needs to be built
            for t in track_lengths:
                seconds.append(seconds[-1] + t[0])
            if seconds[-1] == 2:
                seconds[-1] = 0
            
            cddb.execute("INSERT INTO `vgmdb_CDs` (`CDid`, \
`albumid`, `disc`, `length`, `tracks`) VALUES (NULL, %s, \
%s, %s, %s)", (albumid, disc, seconds[-1], len(seconds[:-1])))

    # Add a few of my own verified albums
    add_sample(cddb, ('4c04ae06', 6633, 1))
    add_sample(cddb, ('770da319', 40, 1))
    add_sample(cddb, ('140d7614', 40, 2))
    add_sample(cddb, ('eb10eb10', 40, 3))
    add_sample(cddb, ('320bbd17', 3443, 1))
    add_sample(cddb, ('590b9a18', 3443, 2))
    add_sample(cddb, ('530bc417', 3443, 3))
    add_sample(cddb, ('8408d80a', 4927, 1))
    add_sample(cddb, ('7b08fe0a', 5974, 1))
    add_sample(cddb, ('bf0e430e', 5863, 1))
    add_sample(cddb, ('be09580d', 5863, 2))
    add_sample(cddb, ('800b300b', 5863, 3))
    add_sample(cddb, ('8508c609', 5112, 1))
    add_sample(cddb, ('900da41c', 3395, 1))
    add_sample(cddb, ('850f771b', 3395, 2))
    add_sample(cddb, ('220ecc15', 3395, 3))
    add_sample(cddb, ('b20f5e1e', 3397, 1))
    add_sample(cddb, ('6710a418', 3397, 2))
    add_sample(cddb, ('3311a914', 3397, 3))
    add_sample(cddb, ('7e05db0a', 8244, 1))
    
    track_lengths.close()
    albums.close()
    cddb.close()
    db.close()
    
