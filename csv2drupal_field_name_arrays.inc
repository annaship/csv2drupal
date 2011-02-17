<?php

function sheet_names() {
  $sheet_names = array(
    'dataset-cck'   => 'dataset',
    'datafile-cck'  => 'datafile',
    'site-cck'      => 'site',
    'person-cck'    => 'person',
    'variable-cck'  => 'variable',
    'keywords'      => 'keywords',
  );
  return $sheet_names;
}

//  $sheet_names = array(
//    'DataSet-CCK'   => 'dataset',
//    'DataFile-CCK'  => 'datafile',
//    'Site-CCK'      => 'site',
//    'Person-CCK'    => 'person',
//    'Variable-CCK'  => 'variable',
//    'Keywords'      => 'keywords',
//  );

function dataset_fields() {
  $dataset_fields = array(
    'field_beg_end_date'              => 'Date Range',
    'field_dataset_abstract'          => 'Abstract',
    'field_dataset_add_info' 				  => 'Additional Information',
    'field_dataset_id' 						    => 'Dataset ID',
    'field_dataset_maintenance'       => 'Maintenance',
    'field_dataset_publication_date'  => 'Publication Date',
    'field_dataset_purpose' 					=> 'Purpose',
    'field_dataset_related_links' 		=> 'Related Links',
    'field_dataset_short_name'    		=> 'Short Name',
    'field_instrumentation'       		=> 'Instrumentation',
    'field_methods' 									=> 'Methods',
    'field_quality' 									=> 'Quality Assurance',
  );
  return $dataset_fields;
}

/*
 Title
Abstract
Short Name
Dataset ID
Purpose
Additional Information
Data File Structure
Related Links
Related Bibliography
Maintenance
Owner
Contact
Data Manager
Field Crew
Lab Crew
Associated Researcher
Publication Date
Site
Date Range
Methods
Instrumentation
Quality Assurance
 */

function dataset_person_ref_fields() {
  $dataset_person_ref_fields = array(
    'field_dataset_owner_ref'        => 'Owner',
    'field_dataset_contact_ref'      => 'Contact',
    'field_dataset_datamanager_ref'  => 'Data Manager',
    'field_dataset_ext_assoc_ref'    => 'Associated Researcher',
    'field_dataset_fieldcrew_ref'    => 'Field Crew',
    'field_dataset_labcrew_ref'      => 'Lab Crew',
  );
  return $dataset_person_ref_fields;
}

function datafile_fields() {
  $datafile_fields = array (
    'field_datafile_description'  => 'Description',
//    'field_data_file'             => 'Data File',
    'field_num_header_line'       => 'Number of Header Lines',
    'field_num_footer_lines'      => 'Number of Footer Lines',
    'field_orientation'           => 'Orientation',
    'field_quote_character'       => 'Quote Character',
    'field_delimiter'             => 'Field Delimiter',
    'field_record_delimiter'      => 'Record Delimiter',
    'field_beg_end_date'          => 'Date Range',
    'field_methods'               => 'Methods',
    'field_instrumentation'       => 'Instrumentation',
    'field_quality'               => 'Quality Assurance',
  );
  return $datafile_fields;
}

function person_fields() {
  $person_fields = array (
    'field_person_first_name'   => 'First Name',
    'field_person_last_name'    => 'Last Name',
    'field_person_organization' => 'Organization',
    'field_person_title'        => 'Title',
    'field_person_address'      => 'Address',
    'field_person_city'         => 'City',
    'field_person_state'        => 'State',
    'field_person_zipcode'      => 'Zip Code',
    'field_person_country'      => 'Country',
    'field_person_phone'        => 'Phone',
    'field_person_fax'          => 'Fax',
    'field_person_email'        => 'Email',
    'field_person_personid'     => 'personid',
    'field_person_role'         => 'Role',
    // 'field_person_uid'          => 'person_uid',
    // 'field_person_pubs'         => 'Publications',
  );
  return $person_fields;
}

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

field_attribute_label_value
field_var_definition_value
field_attribute_unit_value
field_attribute_maximum_value
field_attribute_minimum_value
field_attribute_precision_value
field_attribute_formatstring_value
   *
    'field_attribute_label'         => 'Variable Name',
    'field_var_definition'          => 'Variable Description',
    'field_var_missingvalues'       => 'Missing Value Code',
    'field_attribute_unit'          => 'Units',
    'field_attribute_maximum'       => 'Maximum Value',
    'field_attribute_minimum'       => 'Minimum Value',
    'field_attribute_precision'     => 'Precision',
    'field_attribute_formatstring'  => 'DateTime Format',
    'field_code_definition'         => 'Code Information',

   Name 	Label 	Definition 	Missing Values 	unit 	Maximum Value 	Minimum Value 	precision 	Date Format 	Code-Definition 	Associated Data Files
   */

function variable_fields() {
  $variable_fields = array (
    'field_attribute_label'         => 'Label',
    'field_var_definition'          => 'Definition',
    'field_var_missingvalues'       => 'Missing Values',
    'field_attribute_unit'          => 'unit',
    'field_attribute_maximum'       => 'Maximum Value',
    'field_attribute_minimum'       => 'Minimum Value',
    'field_attribute_precision'     => 'precision',
    'field_attribute_formatstring'  => 'Date Format',
    'field_code_definition'         => 'Code-Definition',
  );
  return $variable_fields;
}

function research_site_fields() {
  $research_site_fields = array (
    'field_research_site_image'       => 'Image',
//    'field_research_site_pt_coords'   => 'Coordinates',
    'field_research_site_elevation'   => 'Elevation',
    'field_research_site_landform'    => 'Landform',
    'field_research_site_geology'     => 'Geology',
    'field_research_site_soils'       => 'Soils',
    'field_research_site_hydrology'   => 'Hydrology',
    'field_research_site_vegetation'  => 'Vegetation',
    'field_research_site_climate'     => 'Climate',
    'field_research_site_history'     => 'History',
    'field_research_site_siteid'      => 'Site ID',

  );
  return $research_site_fields;
}

/*
      $full_name   = $value['First Name'] . " " . $value['Last Name'];
      $node->title = $full_name;
      $node->title = $value['Variable Name'];
      research_site $node->title = $value['Title'];
      data_file $node->title  = $value['Data File Structure Name'];
      data_set $node->title                                      = $value['Title'];

 */

function tags_to_title() {
  $tags_to_title = array (
    'person'        => 'Full Name',
    'variable'      => 'Label',
    'research_site' => 'Title',
    'data_file'     => 'Data File Structure Name',
    'data_set'      => 'Title',
  );
  return $tags_to_title;
}

function forbiddenf_fields() {
  $forbiddenf_fields = array(
    'field_person_phone',
    'field_person_email',
    'field_var_missingvalues',
    'field_code_definition',
    'field_research_site_image',
    'field_beg_end_date',
    'field_methods',
    'field_instrumentation',
    'field_quality',
    'field_dataset_related_links',
  );
  return $forbiddenf_fields;
}
