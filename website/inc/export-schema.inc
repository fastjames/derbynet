<?php
require_once('inc/schema_version.inc');

/* Foreign keys:

   Awards(awardtypeid) -> AwardType
   Awards(rankid) -> Ranks
   Awards(classid) -> Classes
   Awards(racerid) -> RegistrationInfo

   RaceChart(classid) -> Classes
   RaceChart(roundid) -> Rounds
   RaceChart(racerid) -> RegistrationInfo

   Ranks(classid) -> Classes

   RegistrationInfo(classid) -> Classes
   RegistrationInfo(rankid) -> Ranks

   Roster(roundid) -> Rounds
   Roster(classid) -> Classes
   Roster(racerid) -> RegistrationInfo

   Rounds(classid) -> Classes
 */

$schema = array('Classes' => array('classid', 'class'),
                'Ranks' => array('rankid', 'rank', 'classid'),
                'RegistrationInfo' => array('racerid', 'carnumber', 'carname',
                                            'lastname', 'firstname', 'classid', 'rankid',
                                            'passedinspection', 'imagefile', 'exclude'),
                'Rounds' => array('roundid', 'round', 'classid', 'charttype', 'phase'),
                'AwardTypes' => array('awardtypeid', 'awardtype'),
                'Awards' => array('awardid', 'awardname', 'awardtypeid', 'classid', 'rankid',
                                  'racerid', 'sort'),
                'Roster' => array('rosterid', 'roundid', 'classid', 'racerid', 'finalist', 'grandfinalist'),
                'RaceChart' => array('resultid', 'classid', 'roundid', 'heat', 'lane', 'racerid',
                                     'chartnumber', 'finishtime', 'finishplace', 'points',
                                     'completed', 'ignoretime', 'masterheat'),
                'RaceInfo' => array('raceinfoid', 'itemkey', 'itemvalue'),
  );

if (schema_version() >= 2) {
  $schema['Classes'][] = 'sortorder';
  $schema['Ranks'][] = 'sortorder';
  $schema['RegistrationInfo'][] = 'carphoto';
}
?>