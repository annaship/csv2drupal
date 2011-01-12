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

//    dpr($titles);

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
//  dpr($person_nodes);
  $variables_nodes   = create_variable_node($content['variable'], $uid);
//  dpr($variables_nodes);
  $site_nodes        = create_site_node($content['site'], $uid);
//  dpr($site_nodes);
  $datafile_nodes    = create_datafile_node($content['datafile'], $uid);
//  dpr($datafile_nodes);
  $dataset_nodes     = create_dataset_node($content['dataset'], $uid);
//  dpr($dataset_nodes);
  $keywords_taxonomy = create_keywords_node($content['keywords'], $uid);

//  TODO link referrer
}

function get_uid() {
  global $user;
  return $user->uid;
}

function create_values($content) {
  $values = array();
  foreach ($content as $key => $value) {
    foreach ($value as $number => $data) {
      $values[$number][$key] = $data;
    }
  }
  return $values;
}

function create_person_node($content, $uid) {
  dpr("create_person_node");
  $person_values = create_values($content);
  foreach ($person_values as $value) {
    $fields_names = person_fields();
    $node = add_node_value($node, $value, $fields_names);
//    $node->field_person_first_name[0]['value']    = $value['First Name'];
//    $node->field_person_last_name[0]['value']     = $value['Last Name'];
//    $node->field_person_organization[0]['value']  = $value['Organization'];
//    $node->field_person_title[0]['value']         = $value['Title'];
//    $node->field_person_address[0]['value']       = $value['Address'];
//    $node->field_person_city[0]['value']          = $value['City'];
//    $node->field_person_state[0]['value']         = $value['State'];
//    $node->field_person_zipcode[0]['value']       = $value['Zip Code'];
//    $node->field_person_country[0]['value']       = $value['Country'];
//    $node->field_person_phone[0]['value']         = $value['Phone'];
//    $node->field_person_fax[0]['value']           = $value['Fax'];
//    $node->field_person_email[0]['value']         = $value['Email'];
//    $node->field_person_personid[0]['value']      = $value['personid'];
//    $node->field_person_role[0]['value']          = $value['Role'];
//    $node->field_person_uid[0]['value']           = $value['person_uid'];
//    $node->field_person_pubs[0]['value']          = $value['Publications'];
    dpr($node);
  }
}


function create_variable_node($content, $uid) {
  dpr('here in variable');
//  dpr($content);
  $variable_values = create_values($content);
  /*
Variable Name
Variable Description
Units
Measurement Scale
Code Information
Number Type
DateTime Format
Missing Value Code
Missing Value Code Explanation

   */

  foreach ($variable_values as $value) {
//    dpr($value);
    $node = new StdClass();
    $node->type = "variable";
    $node->uid = $uid;
    $node->title = $value['Variable Name'];
    $node->field_attribute_label[0]['value'] = $value['Variable Name'];
    $node->field_var_definition[0]['value'] = $value['Variable Description'];
    $node->field_var_missingvalues[0]['value'] = $value['Missing Value Code'];
    $node->field_attribute_unit[0]['value'] = $value['Units'];
//      $node->field_attribute_maximum[0]['value'] = $value['Maximum Value'];
//      $node->field_attribute_minimum[0]['value'] = $value['Minimum Value'];
//      $node->field_attribute_precision[0]['value'] = $value['precision'];
    $node->field_attribute_formatstring[0]['value'] = $value['DateTime Format'];
    $node->field_code_definition[0]['value'] = $value['Code Information'];
//      $node->field_attribute_assoc_datafile[0]['value'] = $value['Associated Data Files'];
//      TODO node_save here
//      TODO collect nids into array and return it
//  dpr($node);
  }
//  return array of nids
}

function create_site_node($content, $uid) {
//  dpr($content);
  dpr("create_site_node");
  $research_site_values = create_values($content);
  foreach ($research_site_values as $value) {
    $node = new StdClass();
    $node->type = "research_site";
    $node->uid = $uid;
    $node->title = $value['Title'];
    $node->field_research_site_image[0]['value']      = $value['Image'];
    $node->field_research_site_pt_coords[0]['value']  = $value['Coordinates'];
    $node->field_research_site_elevation[0]['value']  = $value['Elevation'];
    $node->field_research_site_landform[0]['value']   = $value['Landform'];
    $node->field_research_site_geology[0]['value']    = $value['Geology'];
    $node->field_research_site_soils[0]['value']      = $value['Soils'];
    $node->field_research_site_hydrology[0]['value']  = $value['Hydrology'];
    $node->field_research_site_vegetation[0]['value'] = $value['Vegetation'];
    $node->field_research_site_climate[0]['value']    = $value['Climate'];
    $node->field_research_site_history[0]['value']    = $value['History'];
    $node->field_research_site_siteid[0]['value']     = $value['Site ID'];
//    dpr($node);
    }
  
}

function add_node_value($node, $value, $fields_names) {
  foreach ($fields_names as $node_field_name => $csv_title) {
    $node->$node_field_name = array();
    if (!empty($value[$csv_title])) $node->$node_field_name = array(array('value' => $value[$csv_title]));
  }
  return $node;
}

function create_datafile_node($content, $uid) {
  dpr("create_datafile_node");
  $data_file_values = create_values($content);
  foreach ($data_file_values as $value) {
//  TODO: collect all variables here
    if (!empty($value['Data File Structure Name'])) {
      $node = new StdClass();
      $node->type = "data_file";
      $node->uid = $uid;
      $node->title = $value['Data File Structure Name'];
      $fields_names = datafile_fields();
      $node = add_node_value($node, $value, $fields_names);
  //    $node->field_datafile_description = array();
  //    if (!empty($value['Description'])) $node->field_datafile_description            = array(array('value' => $value['Description']));
  ////    $node->field_datafile_description[0]['value'] = $value['Description'];
  ////    $node->field_data_file[0]['value']            = $value['Data File'];
  ////    $node +=
  //    $node = add_node_value($node, $value, 'field_data_file');
  ////    echo 'HERE!!';
  ////    dpr($a);
  ////    $node = $node + $a;
  ////    dpr($node);
  ////    $node->field_data_file = array();
  ////    if (!empty($value['Data File'])) $node->field_data_file            = array(array('value' => $value['Data File']));
  //    $node->field_num_header_line[0]['value']      = $value['Number of Header Lines'];
  //    $node->field_num_footer_lines[0]['value']     = $value['Number of Footer Lines'];
  //    $node->field_orientation[0]['value']          = $value['Orientation'];
  //    $node->field_quote_character[0]['value']      = $value['Quote Character'];
  //    $node->field_delimiter[0]['value']            = $value['Field Delimiter'];
  //    $node->field_record_delimiter[0]['value']     = $value['Record Delimiter'];
  //    $node->field_beg_end_date[0]['value']         = $value['Date Range'];
  //    $node->field_methods[0]['value']              = $value['Methods'];
  //    $node->field_instrumentation[0]['value']      = $value['Instrumentation'];
  //    $node->field_quality[0]['value']              = $value['Quality Assurance'];
  //
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



//taking person id (they should be created first)
// TODO the same for files and other refs (bib, sites)
function taking_person_ids($persons) {
  if (isset ($persons)) {
    $persons_arr = array();
    foreach ($persons as $persons_value) {
      $sql = "SELECT nid FROM node WHERE type = 'person' and title = '%s'";
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
  $beg_end_date['value']        = $date_range_values[0];
  $beg_end_date['value2']       = $date_range_values[1];
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

function add_timezone_to_date_arr($date_arr) {
  $date_arr['timezone']     = 'America/New_York';
  $date_arr['timezone_db']  = 'America/New_York';
  $date_arr['date_type']    = 'datetime'; // TODO: or 'date', ask Inigo
  return $date_arr;
}

function create_dataset_node($content, $uid) {
  dpr("create_dataset_node");
    $dataset_values = create_values($content);

    foreach ($dataset_values as $key => $value) {
//      echo "\$key = " . $key;
//      dpr($value);
//  TODO: collect names here (e.g. "Associated Researcher")
      if (!empty($value['Title'])) {
        $node = new StdClass();
        $node->type                                       = "data_set";
        $node->uid                                        = $uid;
        $node->title                                      = $value['Title'];
        $fields_names = dataset_fields();
        $node = add_node_value($node, $value, $fields_names);
        $node->field_beg_end_date[0]                      = create_beg_end_date_arr(convert_date_range($value['Date Range']));
    //    $node->field_dataset_abstract[0]['value']         = $value['Abstract'][0];
    ////    $node->field_dataset_abstract[0]['format']        = '3';
    //    $node->field_dataset_add_info[0]['value']         = $value['Additional Information'][0];
    ////    $node->field_dataset_biblio_ref[0]['value']       = $related_bibliography;
    ////    $node->field_dataset_contact_ref[0]['value']      = $contact;
    ////    $node->field_dataset_datafile_ref[0]['value']     = $data_file_structure;
    ////    $node->field_dataset_datamanager_ref[0]['value']  = $data_manager;
    ////    $node->field_dataset_ext_assoc_ref                = $ext_assoc_arr;
    ////    $node->field_dataset_fieldcrew_ref[0]['value']    = $field_crew;
    //    $node->field_dataset_id[0]['value']               = $value['Dataset ID'][0];
    ////    $node->field_dataset_labcrew_ref[0]['value']      = $lab_crew;
    //    $node->field_dataset_maintenance[0]['value']      = $value['Maintenance'][0];
    ////    $node->field_dataset_maintenance[0]['format']     = '3';
    ////    $node->field_dataset_owner_ref[0]['value']        = $owner;
        $node->field_dataset_publication_date[0]          =  create_single_date_arr($value['Publication Date']);
    //    $node->field_dataset_purpose[0]['value']          = $value['Purpose'][0];
    ////    $node->field_dataset_purpose[0]['format']         = '1';
    //    $node->field_dataset_related_links[0]['value']    = $value['Related Links'][0];
    //    $node->field_dataset_short_name[0]['value']       = $value['Short Name'][0]; //
    ////    $node->field_dataset_site_ref[0]['value']         = $site;
    //    $node->field_instrumentation[0]['value']          = $value['Instrumentation'][0];
    //    $node->field_methods[0]['value']                  = $value['Methods'][0];
    //// TODO add format
    //    $node->field_quality[0]['value']                  = $value['Quality Assurance'][0];
    //// TODO add format
        dpr($node);
      }
    }
}
