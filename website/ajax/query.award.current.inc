<award-presentations>
<?php

// "Current" award being presented, and whether it's revealed
// Cold-cache performance on Mac is ~75ms

require_once('inc/photo-config.inc');
require_once('inc/standings.inc');
require_once('inc/ordinals.inc');

$key = read_raceinfo('current_award_key', '');
$use_subgroups = read_raceinfo_boolean('use-subgroups');
$reveal = read_raceinfo_boolean('reveal_current_award');

// award-<awardid>
// speed-<place>
// speed-<place>-<den>
// speed-<place>-<den>-<rank>
$keys = explode('-', $key);

if (count($keys) >= 2) {
  if ($keys[0] == 'award') {
    $awardid = $keys[1];
    $stmt = $db->prepare('SELECT Awards.racerid, awardname, firstname, lastname,'
                         .' carnumber, carname, rank, imagefile, carphoto'
                         .' FROM '.inner_join('Awards', 'RegistrationInfo',
                                              'Awards.racerid = RegistrationInfo.racerid',
                                              'Ranks', 'Ranks.rankid = RegistrationInfo.rankid')
                         .' WHERE awardid = :awardid');
    $stmt->execute(array(':awardid' => $awardid));
  } else if ($keys[0] == 'speed') {
    // 1st place is 1, 2nd place is 2, etc.
    $place = $keys[1];
    if (count($keys) == 4) {
      $classid = $keys[2];
      $rankid = $keys[3];
      $by_rank = top_finishers_by_rank($place);
      $racerid = $by_rank[$rankid][$place - 1];
      // TODO classname isn't used
      $classname = read_single_value('SELECT class FROM Classes WHERE classid = :classid',
                                     array(':classid' => $classid));
      $rankname = read_single_value('SELECT rank FROM Ranks WHERE rankid = :rankid',
                                    array(':rankid' => $rankid));
      $awardname = nth_fastest($place, $rankname);
    } else if (count($keys) == 3) {
      $classid = $keys[2];
      $by_class = top_finishers_by_class($place);
      $racerid = $by_class[$classid][$place - 1];
      $classname = read_single_value('SELECT class FROM Classes WHERE classid = :classid',
                                     array(':classid' => $classid));
      $awardname = nth_fastest($place, $classname);
    } else {
      $finishers = top_finishers_overall($place);
      $racerid = $finishers[$place - 1];
      $awardname = nth_fastest_in_pack($place);
    }

    if ($racerid > 0) {
      $stmt = $db->prepare('SELECT racerid, :awardname AS awardname, firstname, lastname,'
                           .' carnumber, carname, rank, imagefile, carphoto'
                           .' FROM '.inner_join('RegistrationInfo', 'Ranks',
                                                'Ranks.rankid = RegistrationInfo.rankid')
                           .' WHERE racerid = :racerid');
      $stmt->execute(array(':racerid' => $racerid,
                           ':awardname' => $awardname));
    }
  } else {
    echo "<failure code=\"unrecognized-award-code\">Unrecognized award code</failure>\n";
  }
} else {
  echo "<failure code=\"no-current-award\"/>\n";
}

if (isset($stmt)) {
  foreach ($stmt as $row) {
    echo '<award key="'.$key.'"'
      .' awardname="'.htmlspecialchars($row['awardname'], ENT_QUOTES, 'UTF-8').'"'
      .' reveal="'.($reveal ? "true" : "false").'"'
      .' carnumber="'.htmlspecialchars($row['carnumber'], ENT_QUOTES, 'UTF-8').'"'
      .' carname="'.htmlspecialchars($row['carname'], ENT_QUOTES, 'UTF-8').'"'
      .' firstname="'.htmlspecialchars($row['firstname'], ENT_QUOTES, 'UTF-8').'"'
      .' lastname="'.htmlspecialchars($row['lastname'], ENT_QUOTES, 'UTF-8').'"'
      .($use_subgroups ?
       ' subgroup="'.htmlspecialchars(@$row['rank'], ENT_QUOTES, 'UTF-8').'"' : '')
    .($row['imagefile'] ? ' headphoto="'.headshots()->url_for_racer($row, RENDER_CROPPED).'"' : '')
    .($row['carphoto'] ? ' carphoto="'.car_photo_repository()->url_for_racer($row, RENDER_CROPPED).'"' : '')
    .' duration="'.(microtime(true) - $_SERVER['REQUEST_TIME_FLOAT']).'"'
      .'/>'."\n";
  }
}

?></award-presentations>
