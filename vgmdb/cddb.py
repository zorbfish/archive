#!/usr/local/bin/python
# -*- coding: utf-8 -*-

"""Responds to freedb query and read commands with queries against VGMdb"""

from codecs import register_error
from datetime import date
from os import environ
from string import hexdigits, ascii_letters
from urlparse import parse_qs
import MySQLdb

ARRANGEMENT, REMIX = 2, 32
FRAMES_PER_SECOND = 75
LEADIN = 2 * FRAMES_PER_SECOND

# All language data is located here for easier configuration
JAPANESE_ENCODINGS = {'ja': dict(encoding='Shift-JIS', genre=u'ゲーム',
                                 tracklist_name='Japanese', title_position=1)}
JAPANESE_ENCODINGS['ja.utf8'] = JAPANESE_ENCODINGS['ja'].copy()
JAPANESE_ENCODINGS['ja.utf8'].update(encoding='UTF-8')
ENCODINGS = dict(JAPANESE_ENCODINGS)
ENCODINGS['en'] = dict(encoding='UTF-8', genre='Game',
                       tracklist_name='English', title_position=0)
ENCODINGS['ja-Latn'] = ENCODINGS['en'].copy()
ENCODINGS['ja-Latn'].update(tracklist_name='Romaji', title_position=2)

# VGMdb queries
DISC_READ = """INSERT INTO vgmdb_cddb (mediaid, discid) VALUES (%s, %s) ON
DUPLICATE KEY UPDATE `reads` = `reads` + 1"""

GET_ARTIST_DEFAULT = """SELECT composer, arranger FROM vgmdb_albums WHERE
albumid = %s"""

GET_ARTIST_KANJI = """SELECT role, alias, name, namefamilykanji,
namegivenkanji FROM vgmdb_artists INNER JOIN vgmdb_artists_albums ON
vgmdb_artists.artistid = vgmdb_artists_albums.artistid WHERE
vgmdb_artists_albums.albumid = %s AND role = %s"""

GET_DISC_DETAILS = """SELECT M.albumid, number, tracks, reltimestamp,
parentid, classification FROM vgmdb_albums AS A INNER JOIN vgmdb_media AS M
ON A.albumid = M.albumid WHERE M.mediaid = %s"""

GET_DISC_IDENTITY = """SELECT M.catalog, M.number, COUNT(N.format),
albumtitles FROM vgmdb_media AS M INNER JOIN vgmdb_media AS N ON N.albumid =
M.albumid INNER JOIN vgmdb_albums AS A ON A.albumid = M.albumid WHERE
M.mediaid = %s AND N.format = 0"""

GET_POSSIBLE_MATCHES = """SELECT M.mediaid, M.albumid, parentid, number,
discid, discid_lossy FROM vgmdb_media AS M INNER JOIN vgmdb_albums AS A ON
M.albumid = A.albumid WHERE tracks = %s AND length BETWEEN %s AND %s OR
discid LIKE %s OR discid_lossy = %s"""

GET_TRACKLIST = """SELECT title FROM vgmdb_tracks AS T INNER JOIN
vgmdb_tracklists AS L ON L.tracklistid = T.tracklistid WHERE {0} AND
T.albumid = %s AND disc = %s ORDER BY track ASC"""

# GET_TRACKLIST query modifiers (passed as first argument)
DEFAULT, SPECIFIC = 'L.main = 1', "L.tracklistname LIKE '{0}'"

GET_TRACK_TIMES = """SELECT length FROM vgmdb_tracks AS T INNER JOIN
vgmdb_tracklists AS L ON L.tracklistid = T.tracklistid WHERE L.albumid = %s
AND L.main = 1 AND T.disc = %s AND length != 0 ORDER BY disc, track ASC"""

# Responses
ENDL = '\n'
HTTP_RESPONSE = ENDL.join(['Content-Type: text/plain; charset={0}', '', '{1}', '.'])
COMMAND_SYNTAX_ERROR = u'500 Command syntax error'
COMMAND_UNKNOWN_ERROR = u'500 Unrecognized command'
INTERNAL_SERVER_ERROR = u'530 Internal server error'
MATCH_LINE = u'Soundtrack{0} {1} {2}'
TRACK_LINE = 'TTITLE{0}={1}'.decode('utf-8')
QUERY_FOUND_NO_MATCH = u'202 No match found'
QUERY_FOUND_EXACT_MATCH = u'200 '
QUERY_FOUND_MULTIPLE_MATCHES = "211 Found inexact matches list follows (until terminating marker `.')" + ENDL
READ_FOUND_NO_MATCH = '401 Entry not found'
READ_FOUND_MATCH = ENDL.join(('210 Soundtrack {0}', '# xmcd', '#', \
'# Track frame offsets:', '{1}', '#', '# Disc length: 0 seconds', \
'#', 'DISCID={0}', 'DTITLE={3} / {2}', 'DYEAR={4}', 'DGENRE={5}', \
'{6}', 'EXTD=', '{7}', 'PLAYORDER=')).decode('utf-8')

class CommandArgumentError(Exception):
    pass

def check_discid(discid):
    discid.lower()
    if len(discid) != 8 or discid.strip(hexdigits):
        raise ValueError
    return discid

def get_album_artists(cur, albumid, arrange):
    """Return artist names in requested language if available"""
    
    if cur.language in JAPANESE_ENCODINGS:
        return get_album_artists_kanji(cur, albumid, arrange)

    cur.execute(GET_ARTIST_DEFAULT, (albumid, ))
    composer, arranger = cur.fetchone()
    if arrange:
        return arranger.decode('utf-8')
    return composer.decode('utf-8')

def get_album_artists_kanji(cur, albumid, arrange):
    """Return artist names in Japanese"""
    
    cur.execute(GET_ARTIST_KANJI, (albumid, arrange))
    names = []
    for role, alias, name, familyname, givenname in cur:
        if alias:
            names.append(alias.decode('utf-8'))
        elif not familyname or not givenname:
            names.append(name.decode('utf-8'))
        else:
            names.append((familyname + givenname).decode('utf-8'))
    return u'､'.join(names)

def get_genre(language):
    return ENCODINGS[language]['genre'] # Just one for now

def get_language(uri):
    """Return the encoding and language from the recieved URI"""

    encoding = ENCODINGS['en']['encoding']
    try:
        language = uri.partition('?')[0].strip('/?').split('/')[-1]
        encoding = ENCODINGS[language]['encoding']
    except KeyError:
        language = 'en'
    return encoding, language

def get_title(cur, mediaid):
    """Return a title, including the catalog and disc number if available"""
    
    cur.execute(GET_DISC_IDENTITY, (mediaid, ))
    catalog, disc_number, total_discs, titles = cur.fetchone()
    
    # Omit unknown catalog numbers
    if catalog:
        catalog = '[{0}] '.format(html_to_text(catalog))

    # Return title in requested language if available (defaults to English)
    try:
        titles = titles.splitlines()
        title = titles[ENCODINGS[cur.language]['title_position']]
    except IndexError:
        title = titles[0]

    # Omit disc number for single disc albums
    disc = ''
    if total_discs > 1:
        disc = u' (Disc {0})'.format(disc_number)
        
    return catalog + title.decode('utf-8') + disc

def get_tracklist(cur, parentid, albumid, disc):
    """Return tracklist for a disc"""
    
    cur.execute(GET_TRACKLIST.format(SPECIFIC.format( \
        ENCODINGS[cur.language]['tracklist_name'])), (albumid, disc))
    tracklist = cur.fetchall()
    if not tracklist:
        tracklist = parentid and get_tracklist(cur, 0, parentid, disc)
        if tracklist:
            return tracklist
        cur.execute(GET_TRACKLIST.format(DEFAULT), (albumid, disc))
        tracklist = cur.fetchall()
        if not tracklist:
            return u'' # There is no tracklist
    
    return ENDL.join([TRACK_LINE.format(i, title.decode('utf-8'))
                      for i, (title, ) in enumerate(tracklist)])

def get_tracktimes(offsets, disc_length):
    """Return a list of track times in seconds"""

    # The Redbook specification requires tracks be at least four
    # seconds long. If the first offset is longer than that, the
    # pre-gap contains a track
    if (offsets[0] - LEADIN) > 4 * FRAMES_PER_SECOND:
        offsets.insert(0, LEADIN)

    # The disc's length is used to compute the last track's time
    offsets.append(disc_length * FRAMES_PER_SECOND)

    return map(lambda a, b: (a - b) // 75, offsets[1:], offsets[:-1])

def html_to_text(string):
    """Replace all HTML markup with its plain text equivalent"""
    
    swap = {'&amp;': '&'}
    for k in swap:
        string = string.replace(k, swap[k])
    return string

def query(cur, argv):
    try:
        discid = check_discid(argv[0])
        tracktimes = get_tracktimes(map(int, argv[2:-1]), int(argv[-1]))
    except (IndexError, ValueError):
        raise CommandArgumentError
    return _query(cur, discid, tracktimes)

def _query(cur, discid, tracktimes):
    """Search and return any possible matches for a disc in the database"""

    disclen, trackcount = sum(tracktimes), len(tracktimes)
    cur.execute(GET_POSSIBLE_MATCHES, (trackcount, disclen - trackcount,
                                       disclen + trackcount, discid, discid))
    
    matches = []
    for mediaid, albumid, parentid, disc, lossless, lossy in cur:
        if discid is lossless or discid is lossy:
            matches.append(mediaid)
        else:
            # Possible match; compare its track times
            cur.execute(GET_TRACK_TIMES, (albumid, disc))
            tracks = cur.fetchall()
            if not tracks and parentid:
                cur.execute(GET_TRACK_TIMES, (parentid, disc))
                tracks = cur.fetchall()

            TRACKTIME_TOLERANCE = 3 # seconds
            for i, (track, ) in enumerate(tracks):
                if abs(track - tracktimes[i]) > TRACKTIME_TOLERANCE:
                    break
            else:
                matches.append(mediaid)
                
    response = QUERY_FOUND_NO_MATCH
    if matches:
        response = QUERY_FOUND_EXACT_MATCH
        if len(matches) > 1:
            response = QUERY_FOUND_MULTIPLE_MATCHES
        response += ENDL.join([MATCH_LINE.format(mediaid, discid, \
                        get_title(cur, mediaid)) for mediaid in matches])
    return response

def read(cur, argv):
    try:
        mediaid, discid = argv[0].lower(), check_discid(argv[1])
        mediaid = int(mediaid.lstrip(ascii_letters))
    except (IndexError, ValueError):
        raise CommandArgumentError
    return _read(cur, mediaid, discid)

def _read(cur, mediaid, discid):
    """Convert and return a VGMdb entry as a CDDB/freedb database file"""

    cur.execute(GET_DISC_DETAILS, (mediaid, ))
    disc = cur.fetchone()
    if not disc:
        return READ_FOUND_NO_MATCH
    
    albumid, disc, tracks, timestamp, parentid, albumtype = disc
    arrange = ARRANGEMENT == albumtype == REMIX

    response = READ_FOUND_MATCH.format(discid, \
        ENDL.join(['#\t0' for i in range(tracks)]), get_title(cur, mediaid),
        get_album_artists(cur, parentid or albumid, arrange),
        date.fromtimestamp(timestamp).year, get_genre(cur.language),
        get_tracklist(cur, parentid, albumid, disc),
        ENDL.join(['EXTT{0}='.format(i) for i in range(tracks)]))

    # Increment counter only after successfully building response string
    cur.execute(DISC_READ, (mediaid, discid))
    
    return response

def utf8_to_sjis(error):
    """Replace all UTF-8 codepoints with their Shift-JIS equivalents"""
    
    swap = {u'\uff5e': u'\u301c'}
    try:
        sub = swap[error.object[error.start]]
    except KeyError:
        sub = u'?'
    return sub, error.start + 1

if __name__ == '__main__':
    register_error('utf8-sjis', utf8_to_sjis)
    with MySQLdb.connect(db='vgmdb', user='', passwd='') as cur:
        cur.execute('SET NAMES utf8')
        try:
            # Gather information about the request
            enc, cur.language = ENCODINGS['en']['encoding'], 'en'
            enc, cur.language = get_language(environ['REQUEST_URI'])
            request = parse_qs(environ['QUERY_STRING'])['cmd'][0]
            _, handler, argv = request.split(' ', 2)
            response = locals()[handler](cur, argv.split())
        except CommandArgumentError:
            response = COMMAND_SYNTAX_ERROR
        except KeyError:
            response = COMMAND_UNKNOWN_ERROR
        except Exception:
            response = INTERNAL_SERVER_ERROR
        print HTTP_RESPONSE.format(enc, response.encode(enc, 'utf8-sjis'))
