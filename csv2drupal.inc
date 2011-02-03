<?php

require_once 'includes/PHPExcel/IOFactory.php';
require_once 'csv2drupal_field_name_arrays.inc';
//module_load_include('module', 'geo', 'geo');

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
//  dpr($content);
  return $content;
}

function create_nodes($content, $file_object) {
  $ref_nids = create_person_node($content['person'], &$ref_nids);
  $ref_nids = create_variable_node($content['variable'], &$ref_nids);
  $ref_nids = create_site_node($content['site'], &$ref_nids);
  $ref_nids = create_datafile_node($content['datafile'], &$ref_nids, $file_object);
  $ref_nids = create_dataset_node($content['dataset'], &$ref_nids);
  $keywords_taxonomy = create_keywords_node($content['keywords'], &$ref_nids);
  return $ref_nids;
//  TODO link referrer
}

function get_uid() {
  global $user;
  return $user->uid;
}

//restructures arrays from Excel
function create_values($content) {
  $dataset_person_ref_fields = dataset_person_ref_fields();
  $values = array();
  if ($content) {
    foreach ($content as $key => $value) {
      foreach ($value as $number => $data) {
        if (in_array($key, $dataset_person_ref_fields) AND (preg_match("/,/", $data))) {
          $data = get_person_array($data);
        }
        $values[$number][$key] = $data;
      }
    }
  }
  return $values;
}

function  get_person_array($data) {
  $person_names = array();
  if (preg_match_all("/([^,]+)/", $data, $matches, PREG_SET_ORDER)) {
    foreach ($matches as $match) {
      $person_names[] = trim($match[1]);
    }
  }
  return $person_names;
}


function add_node_value(&$node, $value, $fields_names) {
//  dpm($value);
  foreach ($fields_names as $node_field_name => $csv_title) {
    $node->$node_field_name = array(array('value' => $value[$csv_title]));
  }
  return $node;
}

function taking_person_ids($last_name) {
  unset ($nid_last);
  if (isset ($last_name)) {
    $persons_arr = array();
      $sql_last  = "SELECT nid FROM {content_type_person} WHERE field_person_last_name_value = '%s'";
      $q_last   = db_query($sql_last, $last_name);
        while ($nid_last = db_result($q_last)) {
          $persons_arr[] = $nid_last;
      }
  }
  variable_set('old_person_nids', $persons_arr);
  return $persons_arr;
}

//TODO change format:
/*
dd-mmm-yyyy format
 *  * Publication Date	field_dataset_publication_date	Datetime
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

/*
 function _mymodule_somepage_callback($nid) {
  $query = db_query("SELECT * FROM {mytablename}");

  $data = array();

  $i = 0;
  while ($row = db_fetch_array($query)) {
    $data[$i] = $row;
    $i++;
  }

  $output = theme_table(array('ID', 'Look', 'Taste'), $data);
  return $output;
}

 */

function create_person_node($content, &$ref_nids) {
//  TODO: show Jim
//  dpm('Person content: ');
//  dpm($content);
//  dpm($ref_nids['person']);
//  dpr("create_person_node");
  $persons_arr = array ();
  $person_values = create_values($content);
//  TODO: show Jim
//  dpm('$person_values: ');
//  dpm($person_values);
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
      $persons_arr = taking_person_ids($value['Last Name']);
      $current_destination = drupal_get_destination();
      node_save(&$node);
      $person_nid = $node->nid;
      variable_set('new_person_nid', $person_nid);

      if ($persons_arr) {
//        drupal_set_message('Compare new and old person\'s information');
//        drupal_goto('personell_check');
//      drupal_goto('personell_check', $current_destination);
      }
      else {
//        dpr('no persons');
//        dpm('no persons');
      }

//      TODO get $person_nid from variables
      $ref_nids['person'][] = $person_nid;
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
//      field_attribute_assoc_datafile
      node_save(&$node);
      $ref_nids['variable'][] = $node->nid;
    }
//  dpr($node);
//  dpm($node);
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
      $node        = add_node_value($node, $value, $fields_names);
//      dpr($value['Coordinates']);
//      dpm($value['Coordinates']);
      if (preg_match("/(.+),\s(.+)/", $value['Coordinates'], $matches)) {
        $coord1 = $matches[1];
        $coord2 = $matches[2];
        $coordinates = "POINT(" . $coord1 . " " . $coord2 . ")";
        $q_geo       = db_result(db_query("SELECT GeomFromText('%s')", $coordinates));
//        $q_geo       = db_result(db_query("SELECT GeomFromText('POINT(68.6167 -149.31)')"));
//        $node->field_research_site_pt_coords[0]['geo'] = $coordinates;
//        $node->field_research_site_pt_coords[0]['geo'] = $q_geo;
      }

      /*
       * user warning: Incorrect parameter count in the call to native function 'AsBinary' query: geo_field /* anna : geo_field */
//      SELECT AsBinary() AS geo FROM WHERE vid = 12038 in /Library/WebServer/Documents/drupal6-local/sites/all/modules/geo/modules/geo_field/geo_field.module on line 83.
//       check $coordinates format!!!


//      echo 'HERE!';
//      dpr($coordinates);
//      print $coordinates;
//      $q = db_result(db_query("SELECT PointFromText('%s')", 'POINT(1 1)'));
////      $q = db_result(db_query("SELECT GeomFromText('%s')", 'POINT(31.5 42.2)'));
////      PointFromText(Point(a,b))
////      GeomFromWKB(Point(1,2)
//      $node->field_research_site_pt_coords[0]['geo'] = $q;
//      $wkt = geo_value($q, 'wkt', 'wkb');
//      dpr($wkt);
//      print_r($wkt);
//      parse 'Coordinates' here
      //    'field_research_site_pt_coords'   => 'Coordinates',
//      $node->field_geofield[0]['geo'] = 'POINT(30 30)';
//      $nofe->field_research_site_pt_coords[0]['geo'] = 'POINT(30 30)';
//      $node->field_research_site_pt_coords[0] = array('geo' => '');
//        $wkb = 'POINT(30 30)';
//        $g = GeomFromText($wkb);
//           $geo = geo_load();
//           $point = $geo->point(1, 2);
//
//        dpr($point);
//        dpr($wkb);
//        $geo = geo_load();
//        $geo = geo_load(array('table_name' => $field['geo']));
//          dpr(geo_field_names());
//          echo "UUUU";
//        dpr($geo);
//        dpm($geo);
//        $g = $geo->geomFromWKB($wkb);
//        dpr($g);
//        $wkt = $g->asText();
//
//$wkt = geo('wkt_from_point', 30, 30);
//echo 'UUU';
//geo_value($input, $output_format, $input_format = 'wkb')
//
//$wkt = geo_value($wkt, 'wkt', $gis_input);
//$wkb = geo_value($wkt, 'wkb', 'wkt');

//dpr(geo_list());
//dpm(geo_list());
//dpm(geo_field_names());
//dpr(geo_field_names());

//    [4] => GeoSQLMySQL Object
//        (
//            [gid:protected] => 4
//            [name:protected] => field_research_site_pt_coords
//            [title:protected] => Coordinates
//            [srid:protected] => 4326
//            [geo_type:protected] => 0
//            [table_name:protected] => content_type_research_site
//            [column_name:protected] => field_research_site_pt_coords_geo
//            [schema:protected] =>
//            [indexed:protected] => 1
//            [count:protected] =>
//        )
//
//)
//Array
//(
//    [0] => field_research_site_pt_coords
//)

//      $wkb_data = GeomFromText('POINT(30 30)');
//      dpr($wkb_data);
//      $data = geo_wkb_get_data('POINT(30 30)', 'wkt');
//      echo 'URRA!';
//      dpr($data);
//      print_r($wkb_data);

//      $node->field_research_site_pt_coords[0]['geo'] = $wkt;
//      $node->field_research_site_pt_coords[0]['geo'] = 'POINT(30 30)';
// $node->field_research_site_pt_coords_geo = 'POINT(30 30)';

//    dpr($node);


      node_save(&$node);
      $ref_nids['research_site'][] = $node->nid;
//      $site_nid = $node->nid;
//      $sql = "SELECT X(field_research_site_pt_coords_geo) as longitude,
//             Y(field_research_site_pt_coords_geo) as latitude,
//             AsText(field_research_site_pt_coords_geo) as geo_point
//             FROM {content_type_research_site}
//             WHERE vid=(SELECT max(vid)
//                        FROM {content_type_research_site}
//                        WHERE nid = '%d')";
//
//      $query = db_query($sql, $site_nid);
//      $geo_lon_lat_point =  db_fetch_array($query);
//      dpr($geo_lon_lat_point);
//      dpm($geo_lon_lat_point);

//      select GeomFromText(select GeomFromText('POINT(31.5 42.2)');


//      node_save(&$node);

//      dpr($node);
//      dpm($node);
//      print_r($node);

//      if (is_string($shp['data'])) {
//        $values['geo'] = db_result(db_query("SELECT GeomFromText('%s', %d)", $shp['data'], $srid));
//      } else {
//        $values['geo'] = db_result(db_query("SELECT GeomFromText('%s', %d)", $shp['data']['wkt'], $srid));
//      }


    }
//    dpr($node);
  }
  return $ref_nids;
}

function create_file_object($file) {
  $details = stat($file);
  $filesize = $details['size'];

  // Get the path to your Drupal site's files directory
  $dest = file_directory_path();

  // Copy the file to the Drupal files directory
  if(!file_copy($file, $dest)) {
      echo "Failed to move file: $file.\n";
      return;
  } else {
      // file_move might change the name of the file
      $name = basename($file);
  }

  // Build the file object
  $file_obj = new stdClass();
  $file_obj->filename   = $name;
  $file_obj->filepath   = $file;
  $file_obj->filemime   = file_get_mimetype($name);
  $file_obj->filesize   = $filesize;
  $file_obj->filesource = $name;
  $file_obj->uid        = get_uid();
  $file_obj->status     = FILE_STATUS_TEMPORARY;
  $file_obj->timestamp  = time();
  $file_obj->list       = 1;
  $file_obj->new        = true;

  // Save file to files table
  drupal_write_record('files', $file_obj);
  // change file status to permanent
  file_set_status($file_obj, 1);

  // Attach the file object to your node
//  $node->files[$file_obj->fid] = $file_obj;
//  dpr($file_obj);
//  dpm($file_obj);

  return $file_obj;
}

function create_datafile_node($content, &$ref_nids, $file_object) {
//  dpr("create_datafile_node");
//  dpr($ref_nids);
//  create_file_object();
//    dpr($file_object);
//    dpm($file_object);
  $data_file_values = create_values($content);
  $fields_names     = datafile_fields();
  foreach ($data_file_values as $value) {
    if (!empty($value['Data File Structure Name'])) {
      $node = new StdClass();
      $node->type   = "data_file";
      $node->uid    =  get_uid();
      $node->title  = $value['Data File Structure Name'];
      $node = add_node_value($node, $value, $fields_names);
      $node->field_beg_end_date[0] = create_beg_end_date_arr(convert_date_range($value['Date Range']));
      $node->field_beg_end_date[0]['date_type'] = 'datetime';
      if (!empty($ref_nids['variable'])) {
        foreach ($ref_nids['variable'] as $variable) {
          $node->field_datafile_variable_ref[]['nid'] = $variable;
        }
      }
      if ($ref_nids['research_site']) {
        foreach ($ref_nids['research_site'] as $dataset_site) {
          $node->field_datafile_site_ref[]['nid']     = $dataset_site;
        }
      }

      $node->field_data_file[0] = (array) $file_object;


//      in date:                     [date_type] => date


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
  //  TODO: show Jim
//  dpm('Content: ');
//  dpm($content);
//  dpm($ref_nids['person']);
  $dataset_values = create_values($content);
  $fields_names   = dataset_fields();
  foreach ($dataset_values as $value) {
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
//TODO: take ref for biblio
//    $node->field_dataset_biblio_ref[0]['value']       = $related_bibliography;
      foreach ($ref_nids['data_file'] as $data_file_nid) {
        $node->field_dataset_datafile_ref[]['nid']      = $data_file_nid;
      }
      $dataset_person_refs = get_person_ref($value, $ref_nids['person']);
      $add_person_refs     = add_person_refs(&$node, $dataset_person_refs);
      if ($ref_nids['research_site']) {
        foreach ($ref_nids['research_site'] as $dataset_site) {
          $node->field_dataset_site_ref[]['nid']        = $dataset_site;
        }
      }
      node_save(&$node);
      $ref_nids['data_set'][] = $node->nid;
    }
  }
//  dpm($ref_nids);
  add_referrers($ref_nids);
//  dpr($node);
//  dpm($node);
  return $ref_nids;
}

function add_referrers($ref_nids) {
  /*data_file (Array, 1 element)
0 (String, 5 characters ) 23823
data_set (Array, 1 element)
0 (String, 5 characters ) 23824
   *
   * field_attribute_assoc_datafile
   field_attribute_assoc_datafile (Array, 1 element)
0 (Array, 4 elements)
items (Array, 1 element)
0 (Array, 4 elements)
nid (String, 5 characters ) 18462
vid (String, 5 characters ) 18462
title (String, 30 characters ) El Verde Canopy Arthropodsdata
referrers (Array, 1 element)
field_datafile_variable_ref (String, 27 characters ) field_datafile_variable_ref
limit (Integer) 0
element (Integer) 0
pager (String, 0 characters )
0 (Boolean) FALSE
taxonomy (Array, 0 elements)
files (Array, 0 elements)
   *
   *     [field_attribute_assoc_datafile] => Array
        (
            [0] => Array
                (
                    [items] => Array
                        (
                            [0] => Array
                                (
                                    [nid] => 18462
                                    [vid] => 18462
                                    [title] => El Verde Canopy Arthropodsdata
                                    [referrers] => Array
                                        (
                                            [field_datafile_variable_ref] => field_datafile_variable_ref
                                        )

                                )

                        )

                    [limit] => 0
                    [element] => 0
                    [pager] =&gt;
                )

        )

   */
  $data_set_nids = $ref_nids['data_set'];
  $data_file_nids = $ref_nids['data_file'];
  $variable_nids = $ref_nids['variable'];
  $var_to_datafile_reffers = create_data_file_referrer($data_file_nids);

  if (!empty ($variable_nids)) {
    foreach ($variable_nids as $variable_nid) {
      foreach ($var_to_datafile_reffers as $var_to_datafile_reffer) {
        $var_node = node_load($variable_nid);
        $var_node->field_datafile_variable_ref = $var_to_datafile_reffer;
      }
      node_save(&$var_node);
    }
  }
//  referrer to dataset in file
}

function create_data_file_referrer($data_file_nids) {
  $var_to_datafile_reffers = array();
  if ($data_file_nids) {
    foreach ($data_file_nids as $data_file_nid) {
      $file_node = node_load($data_file_nid);
      $var_to_datafile_reffers[] = array(array(
       'items' => array(array(
         'nid' => $file_node->nid,
         'vid' => $file_node->vid,
         'title' => $file_node->title,
        ))
      ));
    }
  }
  return $var_to_datafile_reffers;
}

function display_report_page()
{
  $csv2drupal_submit_time = variable_get(csv2drupal_submit_time, time());
  $sql = "SELECT nid FROM {node} WHERE created >= %d ORDER BY nid DESC";
  $db_result = db_query($sql, $csv2drupal_submit_time);

  $header       = array('Title', 'Nid');
  $rows['data'] = array();
  while ($res   = db_fetch_object($db_result)) {
    $nid        = $res->nid;
    $node       = node_load($nid);
    $node_nid   = $node->nid;
    $node_title = $node->title;
    $node_type  = $node->type;
    $node_link  = l($nid, 'node/'.$nid);
    if ($node_type != $old_node_type) {
      $rows[] = array(array('data' => 'Type = ' . $node_type, 'colspan' => 2, 'font size' => 4, 'align' => 'center'));
      $old_node_type = $node_type;
    }
    $rows[] = array('Title = ' . $node_title, $node_link);
  }
  if (count($rows) == 1)
      $rows[] = array("No nodes was saved");

  variable_del(csv2drupal_submit_time);
  return theme_table($header, $rows);
}

function  get_person_ref($dataset_value, $person_ref_nids) {
  $dataset_person_refs = array();
  $dataset_person_ref_fields = dataset_person_ref_fields();
  foreach ($dataset_person_ref_fields as $dataset_field_name => $dataset_role) {
    $name = $dataset_value[$dataset_role];
    is_array($name) ? $names = $name : $names = (array) $name;
    foreach ($person_ref_nids as $person_ref_nid) {
      $node = node_load($person_ref_nid);
      $title = $node->title;
      if (in_array(trim($title), $names)) {
        $dataset_person_refs[$dataset_field_name][] = $person_ref_nid;
      }
    }
  }
  return $dataset_person_refs;
}

function add_person_refs(&$node, $dataset_person_refs) {
  foreach ($dataset_person_refs as $field_name => $value1) {
    $refs = array();
    foreach ($value1 as $ref) {
      $refs[]['nid'] = $ref;
    }
    $node->$field_name = $refs;
  }
  return $node;
}
