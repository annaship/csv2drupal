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
        if (!empty($titles[$col])) {
          ${$sheet_names[$loadedSheetName]}[$titles[$col]][] = $res;
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
  $uid               = get_uid();
  $person_nodes      = create_person_node($content['person'], $uid);
  $variables_nodes   = create_variable_node($content['variable'], $uid);
  $site_nodes        = create_site_node($content['site'], $uid);
  $datafile_nodes    = create_datafile_node($content['datafile'], $uid);
  $dataset_nodes     = create_dataset_node($content['dataset'], $uid);
  $keywords_taxonomy = create_keywords_node($content['keywords'], $uid);
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

function create_person_node($content, $uid) {
  dpr("create_person_node");
  $person_values = create_values($content);
  $fields_names  = person_fields();
  foreach ($person_values as $value) {
    if (!empty($value['Last Name'])) {
      $node = new StdClass();
      $node->type  = "person";
      $node->uid   = $uid;
      $node->title = $value['First name'] . " " . $value['Last Name'];
      $node = add_node_value($node, $value, $fields_names);
      $node->field_person_email[0]['email'] = $value['Email'];
      $node->field_person_uid[0]['uid'] = $value['person_uid'];
      $node->field_person_pubs[0]['nid'] = $value['Publications'];
      node_save(&$node);
      echo 'node nid = ';
      $person_nids[] = $node->nid;
//      dpr($node->nid);
    }
    dpr($person_nids);
    dpr($node);
  }
}

function create_variable_node($content, $uid) {
  dpr('here in variable');
  $variable_values = create_values($content);
  $fields_names    = variable_fields();
  foreach ($variable_values as $value) {
    if (!empty($value['Variable Name'])) {
      $node = new StdClass();
      $node->type = "variable";
      $node->uid = $uid;
      $node->title = $value['Variable Name'];
      $node = add_node_value($node, $value, $fields_names);
  //      $node->field_attribute_assoc_datafile[0]['value'] = $value['Associated Data Files'];
  //      TODO node_save here
  //      TODO collect nids into array and return it
    }
//  dpr($node);
//  print_r($node);
  }
//  return array of nids
}

function create_site_node($content, $uid) {
  dpr("create_site_node");
  $research_site_values = create_values($content);
  $fields_names = research_site_fields();
  foreach ($research_site_values as $value) {
    if (!empty($value['Title'])) {
      $node = new StdClass();
      $node->type   = "research_site";
      $node->uid    = $uid;
      $node->title  = $value['Title'];
      $node = add_node_value($node, $value, $fields_names);
    }
//    dpr($node);
  }
}

function create_datafile_node($content, $uid) {
  dpr("create_datafile_node");
  $data_file_values = create_values($content);
  $fields_names = datafile_fields();
  foreach ($data_file_values as $value) {
//  TODO: collect all variables here
    if (!empty($value['Data File Structure Name'])) {
      $node = new StdClass();
      $node->type   = "data_file";
      $node->uid    = $uid;
      $node->title  = $value['Data File Structure Name'];
      $node = add_node_value($node, $value, $fields_names);
      $node->field_beg_end_date[0] = create_beg_end_date_arr(convert_date_range($value['Date Range']));

  //    TODO here should be nids. Add while second opening this node, not now, move into another function
  //    $node->field_datafile_variable_ref[0]['value'] = $value['Variables'];
  //    $node->field_datafile_site_ref[0]['value']    = $value['Sites'];
  //    $node->field_dataset_referrer[0]['value']     = $value['Associated Data Set'];
//      dpr($node);
    }
  }
}

function create_keywords_node($content, $uid) {
    dpr("create_keywords_node");
}

function create_dataset_node($content, $uid) {
  dpr("create_dataset_node");
  $dataset_values = create_values($content);
  $fields_names   = dataset_fields();
  foreach ($dataset_values as $value) {
//  TODO: collect names here (e.g. "Associated Researcher")
    if (!empty($value['Title'])) {
      $node = new StdClass();
      $node->type                                       = "data_set";
      $node->uid                                        = $uid;
      $node->title                                      = $value['Title'];
      $node = add_node_value($node, $value, $fields_names);
      $node->field_beg_end_date[0]                      = create_beg_end_date_arr(convert_date_range($value['Date Range']));
      $node->field_dataset_publication_date[0]          =  create_single_date_arr($value['Publication Date']);
//    $node->field_dataset_biblio_ref[0]['value']       = $related_bibliography;
//    $node->field_dataset_contact_ref[0]['value']      = $contact;
//    $node->field_dataset_datafile_ref[0]['value']     = $data_file_structure;
//    $node->field_dataset_datamanager_ref[0]['value']  = $data_manager;
//    $node->field_dataset_ext_assoc_ref                = $ext_assoc_arr;
//    $node->field_dataset_fieldcrew_ref[0]['value']    = $field_crew;
//    $node->field_dataset_labcrew_ref[0]['value']      = $lab_crew;
//    $node->field_dataset_owner_ref[0]['value']        = $owner;
//    $node->field_dataset_site_ref[0]['value']         = $site;
//      dpr($node);
    }
  }
}
