<?php
/**
 * @file
 * The facetapi_select_options widget plugin class.
 */

/**
 * Widget that renders facets as a select list for a form.
 */
class FacetapiWidgetSelectOptions extends FacetapiWidget {
  /**
   * Overrides FacetapiWidget::__construct().
   *
   * For links, it is better to use the machine name of the facet as opposed to
   * the alias for the key. Alias are usually, but not necessarily, unique. It
   * doesn't make sense to group links in the same element as they are usually
   * rendered in blocks which are separate from one another.
   */
  public function __construct($id, array $realm, FacetapiFacet $facet, stdClass $settings) {
    parent::__construct($id, $realm, $facet, $settings);
    $this->key = $facet['name'];
  }

  /**
   * Implements FacetapiWidget::execute().
   *
   * Transforms the render array into something that can be themed by
   * theme_select().
   *
   * @see FacetapiWidgetSelectOptions::buildOptions()
   */
  public function execute() {
    $element = &$this->build[$this->facet['field alias']];
    // Flatten the element. Selects can't have hierarchy.
    $this->flatten($element, $element);

    // Assemble the options array.
    $selected = '';
    $default_options_label = !empty($this->build['#settings']->settings['default_option_label']) ? $this->build['#settings']->settings['default_option_label'] : t('All');
    $options = ['' => $default_options_label];
    foreach ($element as $key => $item) {
      $options[$item['#indexed_value']] = $item['#markup'];
      if ($item['#active'] === 1) {
        $selected = $item['#indexed_value'];
      }
    }

    // Replace the element with our own render array.
    $element = array(
      '#theme' => 'select',
      '#options' => $options,
      '#attributes' => $this->build['#attributes'],
      '#name' => $this->facet['field alias'],
      '#value' => $selected,
    );

    // Use the pretty paths alias if available.
    $processor = $this->build['#adapter']->getUrlProcessor();
    if ($processor instanceof FacetapiUrlProcessorPrettyPaths) {
      $alias = $processor->getFacetPrettyPathsAlias($this->facet->getFacet());
      if (!empty($alias)) {
        $element['#name'] = $alias;
      }
    }
  }

  /**
   * Flatten hierarchy.
   *
   * All children are added to the element, but not removed from #item_children.
   *
   * If all items in a hierarchy are marked as active, only the lowest level
   * active item will retain that status.
   */
  public function flatten(&$element, $children) {
    $active_found = FALSE;
    foreach ($children as $key => $value) {
      $element[$key] = $value;
      if (!empty($value['#item_children'])) {
        $active = $this->flatten($element, $value['#item_children']);
        $active_found = $active_found ? $active_found : $active;
      }
      if (!empty($value['#active'])) {
        if ($active_found) {
          // We've already found an active item lower down in the tree, so mark
          // this ancestor as inactive.
          $element[$key]['#active'] = 0;
        }
        $active_found = TRUE;
      }
    }
    return $active_found;
  }
}
