<?php
// works with csv file without PHPExcel
function get_info_from_file() {
//  call 5 times for each csv file?
  $filename = get_file_name();
  $content = get_content($filename);

  return $content;
}

function get_file_name() {
//  TODO: form here
//  variable
//  $filename = "/Users/anna/work/drupal/lter/new/today/variable_h.csv";
//  dataset
  $filename = "/Users/anna/work/drupal/lter/new/today/csv_examples_hor/1982_2000gs81tusbm_DataSet.csv";

  return $filename;
}

function get_content($filename) {
  $content = csv_to_array($filename);
  return $content;
}

 function csv_to_array($filename='', $delimiter=',') {
    if(!file_exists($filename) || !is_readable($filename)) {
        return FALSE;
    }

    $header = NULL;
    $data = array();
    if (($handle = fopen($filename, 'r')) !== FALSE)
    {
        while (($row = fgetcsv($handle, 1000, $delimiter)) !== FALSE)
        {
            if(!$header) {
                $header = $row;
                chop_field_names($header);
            }
            else {
                $data[] = array_combine($header, $row);
            }
        }
        fclose($handle);
    }
    return $data;
 }

 function chop_field_names(&$header) {
  foreach ($header as $key => $value) {
    $header[$key] = rtrim($value, " ");
  }
  return $header;
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
