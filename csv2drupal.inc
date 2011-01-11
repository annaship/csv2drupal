<?php

require_once("includes/PHPExcel/IOFactory.php");


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

//function get_info_from_file() {
////  call 5 times for each csv file?
//  $filename = get_file_name();
//  $content = get_content($filename);
//
////  dpr($content);
//  return $content;
//}
//
//function get_file_name() {
////  TODO: form here
////  variable
////  $filename = "/Users/anna/work/drupal/lter/new/today/variable_h.csv";
////  dataset
//  $filename = "/Users/anna/work/drupal/lter/new/today/csv_examples_hor/1982_2000gs81tusbm_DataSet.csv";
//
//  return $filename;
//}
//
//function get_content($filename) {
//  $content = csv_to_array($filename);
//  return $content;
//}

// function csv_to_array($filename='', $delimiter=',') {
//    if(!file_exists($filename) || !is_readable($filename)) {
//        return FALSE;
//    }
//
//    $header = NULL;
//    $data = array();
//    if (($handle = fopen($filename, 'r')) !== FALSE)
//    {
//        while (($row = fgetcsv($handle, 1000, $delimiter)) !== FALSE)
//        {
//            if(!$header) {
//                $header = $row;
//                chop_field_names($header);
//            }
//            else {
//                $data[] = array_combine($header, $row);
//            }
//        }
//        fclose($handle);
//    }
//    return $data;
// }
//
// function chop_field_names(&$header) {
//  foreach ($header as $key => $value) {
//    $header[$key] = rtrim($value, " ");
//  }
//  return $header;
//}

function create_nodes($content) {
//TODO link node references
//  dpr('HERE!!!');
//  dpr($content['variable']);
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
    $node->field_person_first_name[0]['value']    = $value['First Name'];
    $node->field_person_last_name[0]['value']     = $value['Last Name'];
    $node->field_person_organization[0]['value']  = $value['Organization'];
    $node->field_person_title[0]['value']         = $value['Title'];
    $node->field_person_address[0]['value']       = $value['Address'];
    $node->field_person_city[0]['value']          = $value['City'];
    $node->field_person_state[0]['value']         = $value['State'];
    $node->field_person_zipcode[0]['value']       = $value['Zip Code'];
    $node->field_person_country[0]['value']       = $value['Country'];
    $node->field_person_phone[0]['value']         = $value['Phone'];
    $node->field_person_fax[0]['value']           = $value['Fax'];
    $node->field_person_email[0]['value']         = $value['Email'];
    $node->field_person_personid[0]['value']      = $value['personid'];
    $node->field_person_role[0]['value']          = $value['Role'];
    $node->field_person_uid[0]['value']           = $value['person_uid'];
    $node->field_person_pubs[0]['value']          = $value['Publications'];
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
  /*
  Title
  Location
  Image
  Coordinates
  Elevation
  Landform
  Geology
  Soils
  Hydrology
  Vegetation
  Climate
  History
  Site
  ID
   */
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
    dpr($node);
    }
  
}

function create_datafile_node($content, $uid) {
  dpr("create_datafile_node");
}

//function create_dataset_node($content, $uid) {
//    dpr("create_dataset_node");
//    dpr($content);
////TODO: convert [0] into [value] and add parsing where needed
////  $dataset_values = set_dataset_values($content);
////  dpr($dataset_values);
//  $node = put_values_into_node($content, $uid);
//////  dpr($dataset_values);
////
////  dpr($node);
////    return $node;
//
//}

function create_keywords_node($content, $uid) {
    dpr("create_keywords_node");
}

//function set_dataset_values($content) {
//  $dataset_values = array();
//  $dataset_tags   = array(
//    'Abstract',
//    'Additional Information',
//    'Associated Researcher',
//    'Contact',
//    'Data File Structure',
//    'Data Manager',
//    'Dataset ID',
//    'Date Range',
//    'Field Crew',
//    'Instrumentation',
//    'Lab Crew',
//    'Maintenance',
//    'Methods',
//    'Owner',
//    'Publication Date',
//    'Purpose',
//    'Related Bibliography',
//    'Related Links',
//    'Short Name',
//    'Site',
//    'Title',
//    'Quality Assurance',
//  );
//
//  foreach ($content as $value) {
////    TODO take nid for references
//    foreach ($dataset_tags as $dataset_tag) {
//      if (!empty ($value[$dataset_tag])) $dataset_values[$dataset_tag]['value'] = $value[$dataset_tag];
//    }
//  }
//  return $dataset_values;
//}

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

//function put_values_into_node($dataset_values, $uid) {
function create_dataset_node($dataset_values, $uid) {
  
//  print_r("UUU");
//Abstract] => Array
//        (
//            [0
    $node = new StdClass();
    $node->type                                       = "data_set";
    $node->uid                                        = $uid;
    $node->title                                      = $dataset_values['Title'][0];
    $node->field_beg_end_date[0]['value']             = $dataset_values['Date Range'][0]; 
// TODO parse and format date
//    [field_beg_end_date] => Array
//        (
//            [0] => Array
//                (
//                    [value] => 2009-02-15T00:00:00
//                    [value2] => 2009-02-15T00:00:00
//                    [timezone] => America/New_York
//                    [timezone_db] => America/New_York
//                    [date_type] => date
//                )
//
//        )
//    [field_beg_end_date] => Array
//        (
//            [0] => Array
//                (
//                    [0] => 6/1/1982 - 7-31-2010
//                )
//
//        )
    $node->field_dataset_abstract[0]['value']         = $dataset_values['Abstract'][0];
//    TODO add format
//    field_dataset_abstract] => Array
//        (
//            [0] => Array
//                (
//                    [value] => Let...
//
//                    [format] => 3
//                )
//
//        )       



    $node->field_dataset_add_info[0]['value']         = $dataset_values['Additional Information'][0];
//    $node->field_dataset_biblio_ref[0]['value']       = $related_bibliography; 
//    $node->field_dataset_contact_ref[0]['value']      = $contact; 
//    $node->field_dataset_datafile_ref[0]['value']     = $data_file_structure; 
//    $node->field_dataset_datamanager_ref[0]['value']  = $data_manager; 
//    $node->field_dataset_ext_assoc_ref                = $ext_assoc_arr; 
//    $node->field_dataset_fieldcrew_ref[0]['value']    = $field_crew; 
    $node->field_dataset_id[0]['value']               = $dataset_values['Dataset ID'][0]; 
//    $node->field_dataset_labcrew_ref[0]['value']      = $lab_crew; 
    $node->field_dataset_maintenance[0]['value']      = $dataset_values['Maintenance'][0];
//    TODO add format
//    [field_dataset_maintenance] => Array
//        (
//            [0] => Array
//                (
//                    [value] => Damnum veniam singularis verto adipiscing. Laoreet euismod hos. Autem neque olim persto. Huic euismod abbas te vel metuo laoreet zelus te os. Validus sagaciter laoreet populus te ea enim. Olim verto paulatim tego melior. Cogo singularis sit. Facilisis tation erat vindico autem. Suscipit enim iustum pala iusto eligo fere ille eligo ideo. Causa quidem pertineo lobortis dignissim euismod aptent.
//
//Scisco luptatum pala erat olim i...
//                    [format] => 3
//                )
//
//        )

//    $node->field_dataset_owner_ref[0]['value']        = $owner; 
    $node->field_dataset_publication_date[0]['value']   =  $dataset_values['Publication Date'][0];
//  TODO  add fields
//        [field_dataset_publication_date] => Array
//        (
//            [0] => Array
//                (
//                    [value] => 2008-03-09 11:27:12
//                    [timezone] => America/New_York
//                    [timezone_db] => America/New_York
//                    [date_type] => datetime
//                )
//
//        )

    $node->field_dataset_purpose[0]['value']          = $dataset_values['Purpose'][0];
//  TODO add [format] => 1
    $node->field_dataset_related_links[0]['value']    = $dataset_values['Related Links'][0]; 
    //TODO all empty should not have [0]! check above
//        [field_dataset_related_links] => Array
//        (
//        )
//    [field_dataset_related_links] => Array
//        (
//            [0] =>
//        )

    $node->field_dataset_short_name[0]['value']       = $dataset_values['Short Name'][0]; // 
//    $node->field_dataset_site_ref[0]['value']         = $site; 
    $node->field_instrumentation[0]['value']          = $dataset_values['Instrumentation'][0]; 
    $node->field_methods[0]['value']                  = $dataset_values['Methods'][0]; 
// TODO add format
    $node->field_quality[0]['value']                  = $dataset_values['Quality Assurance'][0];
// TODO add format
//    dpr($node);
}

