<?php

require_once 'includes/PHPExcel/IOFactory.php';
require_once 'csv2drupal_field_name_arrays.inc';

function get_data_from_excel_file($file) {
  $sheet_names = array(
    'DataSet-csv'   => 'dataset',
    'DataFile-csv'  => 'datafile',
    'Site-csv'      => 'site',
    'Person-csv'    => 'person',
    'Variable-csv'  => 'variable',
    'Keywords'      => 'keywords',
  );

  $objReader = PHPExcel_IOFactory::createReaderForFile($file);
  $objReader->setLoadSheetsOnly(0);
  $objReader->setReadDataOnly(true);
  $objPHPExcel = PHPExcel_IOFactory::load($file);

  $loadedSheetNames = $objPHPExcel->getSheetNames();
  foreach ($loadedSheetNames as $sheetIndex => $loadedSheetName) {
    $objWorksheet = $objPHPExcel->setActiveSheetIndex($sheetIndex);
    $dim = $objWorksheet->calculateWorksheetDimension();
    $highestRow = $objWorksheet->getHighestRow();
    $highestColumn = $objWorksheet->getHighestColumn();
    $highestColumnIndex = PHPExcel_Cell::columnIndexFromString($highestColumn);

    //get titles
    $titles = array();
    for ($col = 0; $col <= $highestColumnIndex; ++$col) {
      $titles[] = trim($objWorksheet->getCellByColumnAndRow($col, "1")->getValue());
    }

    for ($row = 2; $row <= $highestRow; ++$row) {
      for ($col = 0; $col <= $highestColumnIndex; ++$col) {
        $res = $objWorksheet->getCellByColumnAndRow($col, $row)->getValue();
        if (!empty($loadedSheetName)) {
          if (!empty($titles[$col])) {
            ${$sheet_names[$loadedSheetName]}[$titles[$col]][] = $res;
          }
        }
      }
    }
    if (in_array($sheet_names[$loadedSheetName], $sheet_names)) {
      $content[$sheet_names[$loadedSheetName]] = ${$sheet_names[$loadedSheetName]};
    }
  }
  return $content;
}

function create_nodes($content) {
//TODO link node references
  $ref_nids = create_person_node($content['person'], &$ref_nids);
//  echo 'in create_node: person';
  $ref_nids = create_variable_node($content['variable'], &$ref_nids);
  $ref_nids = create_site_node($content['site'], &$ref_nids);
  $ref_nids = create_datafile_node($content['datafile'], &$ref_nids);
//  echo 'in create_node: file';
  $ref_nids = create_dataset_node($content['dataset'], &$ref_nids);
  $keywords_taxonomy = create_keywords_node($content['keywords'], &$ref_nids);
//  dpr($ref_nids);
  return $ref_nids;
//  TODO link referrer
}

function get_uid() {
  global $user;
  return $user->uid;
}

//restructure arrays from Excel
function create_values($content) {
  $values = array();
  foreach ($content as $key => $value) {
    foreach ($value as $number => $data) {
      $values[$number][$key] = $data;
    }
  }
  return $values;
}

function add_node_value(&$node, $value, $fields_names) {
  foreach ($fields_names as $node_field_name => $csv_title) {
    $node->$node_field_name = array(array('value' => $value[$csv_title]));
  }
  return $node;
}

// taking person id (they should be created first)
// TODO the same for files and other refs (bib, sites)
function taking_person_ids($persons) {
  if (isset ($persons)) {
    $persons_arr = array();
    foreach ($persons as $persons_value) {
      $sql = "SELECT nid FROM {node} WHERE type = 'person' and title = '%s'";
      $q = db_query($sql, $persons_value);
        while ($nid = db_result($q)) {
        $persons_arr[][nid] = $nid;
      }
    }
    return $persons_arr;
  }
}

//function convert_date_range($date_range) {
//  if (preg_match_all("/(\d{1,2}).(\d{1,2}).(\d{2,4})/", $date_range, $matches, PREG_SET_ORDER)) {
//    foreach ($matches as $match) {
//        $date = str_replace('-', '/', $match[0]);
//        $dates[] = strftime("%G-%m-%dT00:00:00", strtotime($date));
//    }
//  }
//  return $dates;
//}

//TODO change format:
/*
 * Publication Date	field_dataset_publication_date	Datetime
 * Date Range	field_beg_end_date	Date
 *
 *  */

//TODO: add check for single vs. range
function convert_date_range($date_range) {
  if (preg_match_all("/(\d{1,2}).(\d{1,2}).(\d{2,4})/", $date_range, $matches, PREG_SET_ORDER)) {
    foreach ($matches as $match) {
      $date = str_replace('-', '/', $match[0]);
      $dates[] = strftime("%G-%m-%d 00:00:00", strtotime($date));
    }
  }
  elseif (preg_match_all("/^(\d{4})$/", $date_range, $matches, PREG_SET_ORDER)) {
    foreach ($matches as $match) {
      $date = preg_replace("/^(\d{4})$/", "12/12/$1", $match[0]);
      $dates[] = strftime("%G-%m-%d 00:00:00", strtotime($date));
    }
  }
//  dpr($dates);
  return $dates;
}

function create_beg_end_date_arr($date_range_values) {
  $beg_end_date = array();
  $beg_end_date['value']  = $date_range_values[0];
  $beg_end_date['value2'] = $date_range_values[1];
  $beg_end_date += add_timezone_to_date_arr($beg_end_date);
  return $beg_end_date;
}

function create_single_date_arr($date_value) {
  $date_arr = array();
  $converted_value = convert_date_range($date_value);
  $date_arr['value'] = $converted_value[0];
  $date_arr += add_timezone_to_date_arr($date_arr);
  return $date_arr;
}

function add_timezone_to_date_arr(&$date_arr) {
  $date_arr['timezone']     = 'America/New_York';
  $date_arr['timezone_db']  = 'America/New_York';
  $date_arr['date_type']    = 'datetime'; // TODO: or 'date', ask Inigo
  return $date_arr;
}

function create_person_node($content, &$ref_nids) {
//  dpr("create_person_node");
  $person_values = create_values($content);
  $fields_names  = person_fields();
  foreach ($person_values as $value) {
    if (!empty($value['Last Name'])) {
      $node = new StdClass();
      $node->type  = "person";
      $node->uid   =  get_uid();
      $full_name   = $value['First Name'] . " " . $value['Last Name'];
      $node->title = $full_name;
      $node = add_node_value($node, $value, $fields_names);
      $node->field_person_email[0]['email'] = $value['Email'];
      $node->field_person_uid[0]['uid'] = $value['person_uid'];
      $node->field_person_pubs[0]['nid'] = $value['Publications'];
      node_save(&$node);
      $ref_nids['person'][] = $node->nid;
    }
//    dpr($node);
  }
//  dpr($ref_nids);
  return $ref_nids;
}

function create_variable_node($content, &$ref_nids) {
//  dpr('here in variable');
  $variable_values = create_values($content);
  $fields_names    = variable_fields();
  foreach ($variable_values as $value) {
    if (!empty($value['Variable Name'])) {
      $node        = new StdClass();
      $node->type  = "variable";
      $node->uid   =  get_uid();
      $node->title = $value['Variable Name'];
      $node = add_node_value($node, $value, $fields_names);
  //      $node->field_attribute_assoc_datafile[0]['value'] = $value['Associated Data Files'];
  //      TODO node_save here
  //      TODO collect nids into array and return it
      node_save(&$node);
      $ref_nids['variable'][] = $node->nid;
    }
//  dpr($node);
//  print_r($node);
  }
  return $ref_nids;
}

function create_site_node($content, &$ref_nids) {
//  dpr("create_site_node");
  $research_site_values = create_values($content);
  $fields_names = research_site_fields();
  foreach ($research_site_values as $value) {
    if (!empty($value['Title'])) {
      $node = new StdClass();
      $node->type  = "research_site";
      $node->uid   =  get_uid();
      $node->title = $value['Title'];
      $node = add_node_value($node, $value, $fields_names);
//      node_save(&$node);
      $ref_nids['research_site'][] = $node->nid;
    }
//    dpr($node);
  }
  return $ref_nids;
}

function create_datafile_node($content, &$ref_nids) {
//  dpr("create_datafile_node");
//  dpr($ref_nids);
  $data_file_values = create_values($content);
  $fields_names = datafile_fields();
  foreach ($data_file_values as $value) {
    if (!empty($value['Data File Structure Name'])) {
      $node = new StdClass();
      $node->type   = "data_file";
      $node->uid    =  get_uid();
      $node->title  = $value['Data File Structure Name'];
      $node = add_node_value($node, $value, $fields_names);
      $node->field_beg_end_date[0] = create_beg_end_date_arr(convert_date_range($value['Date Range']));
      $node->field_beg_end_date[0]['date_type'] = 'datetime';
      foreach ($ref_nids['variable'] as $variable) {
        $node->field_datafile_variable_ref[]['nid'] = $variable;
      }
      if ($ref_nids['research_site']) {
        foreach ($ref_nids['research_site'] as $dataset_site) {
          $node->field_datafile_site_ref[]['nid']     = $dataset_site;
        }
      }

//      in date:                     [date_type] => date

//
      /*
[field_data_file] => Array
(
[0] => Array
  (
      [fid] => 42
      [list] => 1
      [data] => Array
          (
              [fid] => 42
              [width] => 0
              [height] => 0
              [duration] => 0
              [audio_format] =>
              [audio_sample_rate] => 0
              [audio_channel_mode] =>
              [audio_bitrate] => 0
              [audio_bitrate_mode] =>
              [tags] => Array
                  (
                  )

          )

      [uid] => 1
      [filename] => nodes_test.csv
      [filepath] => sites/default/files/nodes_test.csv
      [filemime] => text/csv
      [filesize] => 225
      [status] => 1
      [timestamp] => 1281454967
      [origname] =>
  )

)

       */

//      $node->field_dataset_referrer[0]['items'][0]['nid']      = '4255';
//      foreach ($ref_nids['research_site'] as $dataset_site) {
//        $node->field_datafile_site_ref[]['nid']     = $dataset_site;
//      }
//      $node->field_dataset_referrer[]['nid']      = $dataset_site;
//field_dataset_referrer] => Array
//        (
//            [0] => Array
//                (
//                    [items] => Array
//                        (
//                            [0] => Array
//                                (
//                                    [nid] => 2775

//    $node->field_dataset_referrer[0]['value']     = $value['Associated Data Set'];
//$ref_nids['data_set']
/*
 *             [0] => Array
                (
                    [items] => Array
                        (
                            [0] => Array
                                (
                                    [nid] => 2775
                                    [vid] => 2775
                                    [title] => dataset1
                                    [referrers] => Array
                                        (
                                            [field_dataset_datafile_ref] => field_dataset_datafile_ref
                                        )

                                )

                        )

                    [limit] => 3
                    [element] => 1
                    [pager] => 
                )

        )

 */
      node_save(&$node);
      $ref_nids['data_file'][] = $node->nid;
//      dpr($node);
    }
  }
//  dpr($ref_nids);
  return $ref_nids;
}

function create_keywords_node($content, &$ref_nids) {
//    dpr("create_keywords_node");
    return $ref_nids;
}

function create_dataset_node($content, &$ref_nids) {
//  dpr("create_dataset_node");
  $dataset_values = create_values($content);
  $fields_names   = dataset_fields();
  foreach ($dataset_values as $value) {
//  TODO: collect names here (e.g. "Associated Researcher")
    if (!empty($value['Title'])) {
      $node = new StdClass();
      $node->type                                       = "data_set";
      $node->uid                                        =  get_uid();
      $node->title                                      = $value['Title'];
      $node = add_node_value($node, $value, $fields_names);
      $node->field_beg_end_date[0]                      = create_beg_end_date_arr(convert_date_range($value['Date Range']));
      $node->field_beg_end_date[0]['date_type']         = 'datetime';
      $node->field_dataset_publication_date[0]          = create_single_date_arr($value['Publication Date']);
      $node->field_dataset_publication_date[0]['date_type'] = 'date';

//    $node->field_dataset_biblio_ref[0]['value']       = $related_bibliography;
//    $node->field_dataset_contact_ref[0]['value']      = $contact;
      foreach ($ref_nids['data_file'] as $data_file_nid) {
        $node->field_dataset_datafile_ref[]['nid']      = $data_file_nid;
      }
//    $node->field_dataset_datamanager_ref[0]['value']  = $data_manager;
//    $node->field_dataset_ext_assoc_ref                = $ext_assoc_arr;
//    $node->field_dataset_fieldcrew_ref[0]['value']    = $field_crew;
//    $node->field_dataset_labcrew_ref[0]['value']      = $lab_crew;
//    $node->field_dataset_owner_ref[0]['value']        = $owner;
      if ($ref_nids['research_site']) {
        foreach ($ref_nids['research_site'] as $dataset_site) {
          $node->field_dataset_site_ref[]['nid']      = $dataset_site;
        }
      }
      node_save(&$node);
      $ref_nids['data_set'][] = $node->nid;
    }
  }
//  dpr($node);
  return $ref_nids;
}

function show_title_and_number_of_nodes($nids) {
  if (!empty($nids)) {
    foreach ($nids as $key => $nid_array) {
//      dpr($nids);
//      echo '$key = ' . $key . '\n<br/>';
      drupal_set_message($key);
      foreach ($nid_array as $nid) {
//        if (is_int($nid)) {
          $node = node_load($nid);
          $node_nid = $node->nid;
          $node_title = $node->title;
          drupal_set_message('Title =' . $node_title . ', nid = ' . $node_nid);
//        }
      }
//      echo 'nid =';
//      dpr($nid);
    }
  }
}